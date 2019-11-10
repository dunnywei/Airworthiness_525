https://www.linuxjournal.com/article/6735
Introduction to Sound Programming with ALSA
Audio/Video
by Jeff Tranter on September 30, 2004
ALSA stands for the Advanced Linux Sound Architecture. It consists of a set of kernel drivers, an application programming interface (API) library and utility programs for supporting sound under Linux. In this article, I present a brief overview of the ALSA Project and its software components. The focus is on programming the PCM interfaces of ALSA, including programming examples with which you can experiment.

You may want to explore ALSA simply because it is new, but it is not the only sound API available. ALSA is a good choice if you are performing low-level audio functions for maximum control and performance or want to make use of special features not supported by other sound APIs. If you already have written an audio application, you may want to add native support for the ALSA sound drivers. If your primary interest isn't audio and you simply want to play sound files, using one of the higher-level sound toolkits, such as SDL, OpenAL or those provided in desktop environments, may be a better choice. By using ALSA you are restricted to using systems running a Linux kernel with ALSA support.

History of ALSA
The ALSA Project was started because the sound drivers in the Linux kernel (OSS/Free drivers) were not being maintained actively and were lagging behind the capabilities of new sound technology. Jaroslav Kysela, who previously had written a sound card driver, started the project. Over time, more developers joined, support for many sound cards was added and the structure of the API was refined.

During development of the 2.5 series of Linux kernel, ALSA was merged into the official kernel source. With the release of the 2.6 kernel, ALSA will be part of the stable Linux kernel and should be in wide use.

Digital Audio Basics
Sound, consisting of waves of varying air pressure, is converted to its electrical form by a transducer, such as a microphone. An analog-to-digital converter (ADC) converts the analog voltages into discrete values, called samples, at regular intervals in time, known as the sampling rate. By sending the samples to a digital-to-analog converter and an output transducer, such as a loudspeaker, the original sound can be reproduced.

The size of the samples, expressed in bits, is one factor that determines how accurately the sound is represented in digital form. The other major factor affecting sound quality is the sampling rate. The Nyquist Theorem states that the highest frequency that can be represented accurately is at most one-half the sampling rate.

ALSA Basics
ALSA consists of a series of kernel device drivers for many different sound cards, and it also provides an API library, libasound. Application developers are encouraged to program using the library API and not the kernel interface. The library provides a higher-level and more developer-friendly programming interface along with a logical naming of devices so that developers do not need to be aware of low-level details such as device files.

In contrast, OSS/Free drivers are programmed at the kernel system call level and require the developer to specify device filenames and perform many functions using ioctl calls. For backward compatibility, ALSA provides kernel modules that emulate the OSS/Free sound drivers, so most existing sound applications continue to run unchanged. An emulation wrapper library, libaoss, is available to emulate the OSS/Free API without kernel modules.

ALSA has a capability called plugins that allows extension to new devices, including virtual devices implemented entirely in software. ALSA provides a number of command-line utilities, including a mixer, sound file player and tools for controlling special features of specific sound cards.

ALSA Architecture
The ALSA API can be broken down into the major interfaces it supports:

Control interface: a general-purpose facility for managing registers of sound cards and querying the available devices.

PCM interface: the interface for managing digital audio capture and playback. The rest of this article focuses on this interface, as it is the one most commonly used for digital audio applications.

Raw MIDI interface: supports MIDI (Musical Instrument Digital Interface), a standard for electronic musical instruments. This API provides access to a MIDI bus on a sound card. The raw interface works directly with the MIDI events, and the programmer is responsible for managing the protocol and timing.

Timer interface: provides access to timing hardware on sound cards used for synchronizing sound events.

Sequencer interface: a higher-level interface for MIDI programming and sound synthesis than the raw MIDI interface. It handles much of the MIDI protocol and timing.

Mixer interface: controls the devices on sound cards that route signals and control volume levels. It is built on top of the control interface.

Device Naming
The library API works with logical device names rather than device files. The device names can be real hardware devices or plugins. Hardware devices use the format hw:i,j, where i is the card number and j is the device on that card. The first sound device is hw:0,0. The alias default refers to the first sound device and is used in all of the examples in this article. Plugins use other unique names; plughw:, for example, is a plugin that provides access to the hardware device but provides features, such as sampling rate conversion, in software for hardware that does not directly support it. The dmix and dshare plugins allow you to downmix several streams and split a single stream dynamically among different applications.

Sound Buffers and Data Transfer
A sound card has a hardware buffer that stores recorded samples. When the buffer is sufficiently full, it generates an interrupt. The kernel sound driver then uses direct memory access (DMA) to transfer samples to an application buffer in memory. Similarly, for playback, another application buffer is transferred from memory to the sound card's hardware buffer using DMA.

These hardware buffers are ring buffers, meaning the data wraps back to the start when the end of the buffer is reached. A pointer is maintained to keep track of the current positions in both the hardware buffer and the application buffer. Outside of the kernel, only the application buffer is of interest, so from here on we discuss only the application buffer.

The size of the buffer can be programmed by ALSA library calls. The buffer can be quite large, and transferring it in one operation could result in unacceptable delays, called latency. To solve this, ALSA splits the buffer up into a series of periods (called fragments in OSS/Free) and transfers the data in units of a period.

A period stores frames, each of which contains the samples captured at one point in time. For a stereo device, the frame would contain samples for two channels. Figure 1 illustrates the breakdown of a buffer into periods, frames and samples with some hypothetical values. Here, left and right channel information is stored alternately within a frame; this is called interleaved mode. A non-interleaved mode, where all the sample data for one channel is stored followed by the data for the next channel, also is supported.


Figure 1. The Application Buffer

Over and Under Run
When a sound device is active, data is transferred continuously between the hardware and application buffers. In the case of data capture (recording), if the application does not read the data in the buffer rapidly enough, the circular buffer is overwritten with new data. The resulting data loss is known as overrun. During playback, if the application does not pass data into the buffer quickly enough, it becomes starved for data, resulting in an error called underrun. The ALSA documentation sometimes refers to both of these conditions using the term XRUN. Properly designed applications can minimize XRUN and recover if it occurs.

A Typical Sound Application
Programs that use the PCM interface generally follow this pseudo-code:

open interface for capture or playback
set hardware parameters
(access mode, data format, channels, rate, etc.)
while there is data to be processed:
   read PCM data (capture)
   or write PCM data (playback)
close interface
We look at some working code in the following sections. I recommend you compile and run these on your Linux system, look at the output and try some of the suggested modifications. The full listings for the example programs that accompany this article are available for download from ftp.linuxjournal.com/pub/lj/listings/issue126/6735.tgz.

Listing 1. Display Some PCM Types and Formats


#include <alsa/asoundlib.h>

int main() {
  int val;

  printf("ALSA library version: %s\n",
          SND_LIB_VERSION_STR);

  printf("\nPCM stream types:\n");
  for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
    printf("  %s\n",
      snd_pcm_stream_name((snd_pcm_stream_t)val));

  printf("\nPCM access types:\n");
  for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
    printf("  %s\n",
      snd_pcm_access_name((snd_pcm_access_t)val));

  printf("\nPCM formats:\n");
  for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
    if (snd_pcm_format_name((snd_pcm_format_t)val)
      != NULL)
      printf("  %s (%s)\n",
        snd_pcm_format_name((snd_pcm_format_t)val),
        snd_pcm_format_description(
                           (snd_pcm_format_t)val));

  printf("\nPCM subformats:\n");
  for (val = 0; val <= SND_PCM_SUBFORMAT_LAST;
       val++)
    printf("  %s (%s)\n",
      snd_pcm_subformat_name((
        snd_pcm_subformat_t)val),
      snd_pcm_subformat_description((
        snd_pcm_subformat_t)val));

  printf("\nPCM states:\n");
  for (val = 0; val <= SND_PCM_STATE_LAST; val++)
    printf("  %s\n",
           snd_pcm_state_name((snd_pcm_state_t)val));

  return 0;
}


Listing 1 displays some of the PCM data types and parameters used by ALSA. The first requirement is to include the header file that brings in the definitions for all of the ALSA library functions. One of the definitions is the version of ALSA, which is displayed.

The remainder of the program iterates through a number of PCM data types, starting with the stream types. ALSA provides symbolic names for the last enumerated value and a utility function that returns a descriptive string for a value. As you can see in the output, ALSA supports many different data formats, 38 for the version of ALSA on my system.

The program must be linked with the ALSA library, libasound, to run. Typically, you would add the option -lasound on the linker command line. Some ALSA library functions use the dlopen function and floating-point operations, so you also may need to add -ldl and -lm.

Listing 2. Opening PCM Device and Setting Parameters


/*

This example opens the default PCM device, sets
some parameters, and then displays the value
of most of the hardware parameters. It does not
perform any sound playback or recording.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

int main() {
  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle,
                                 params, &val, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Display information about the PCM interface */

  printf("PCM handle name = '%s'\n",
         snd_pcm_name(handle));

  printf("PCM state = %s\n",
         snd_pcm_state_name(snd_pcm_state(handle)));

  snd_pcm_hw_params_get_access(params,
                          (snd_pcm_access_t *) &val);
  printf("access type = %s\n",
         snd_pcm_access_name((snd_pcm_access_t)val));

  snd_pcm_hw_params_get_format(params, &val);
  printf("format = '%s' (%s)\n",
    snd_pcm_format_name((snd_pcm_format_t)val),
    snd_pcm_format_description(
                             (snd_pcm_format_t)val));

  snd_pcm_hw_params_get_subformat(params,
                        (snd_pcm_subformat_t *)&val);
  printf("subformat = '%s' (%s)\n",
    snd_pcm_subformat_name((snd_pcm_subformat_t)val),
    snd_pcm_subformat_description(
                          (snd_pcm_subformat_t)val));

  snd_pcm_hw_params_get_channels(params, &val);
  printf("channels = %d\n", val);

  snd_pcm_hw_params_get_rate(params, &val, &dir);
  printf("rate = %d bps\n", val);

  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  printf("period time = %d us\n", val);

  snd_pcm_hw_params_get_period_size(params,
                                    &frames, &dir);
  printf("period size = %d frames\n", (int)frames);

  snd_pcm_hw_params_get_buffer_time(params,
                                    &val, &dir);
  printf("buffer time = %d us\n", val);

  snd_pcm_hw_params_get_buffer_size(params,
                         (snd_pcm_uframes_t *) &val);
  printf("buffer size = %d frames\n", val);

  snd_pcm_hw_params_get_periods(params, &val, &dir);
  printf("periods per buffer = %d frames\n", val);

  snd_pcm_hw_params_get_rate_numden(params,
                                    &val, &val2);
  printf("exact rate = %d/%d bps\n", val, val2);

  val = snd_pcm_hw_params_get_sbits(params);
  printf("significant bits = %d\n", val);

  snd_pcm_hw_params_get_tick_time(params,
                                  &val, &dir);
  printf("tick time = %d us\n", val);

  val = snd_pcm_hw_params_is_batch(params);
  printf("is batch = %d\n", val);

  val = snd_pcm_hw_params_is_block_transfer(params);
  printf("is block transfer = %d\n", val);

  val = snd_pcm_hw_params_is_double(params);
  printf("is double = %d\n", val);

  val = snd_pcm_hw_params_is_half_duplex(params);
  printf("is half duplex = %d\n", val);

  val = snd_pcm_hw_params_is_joint_duplex(params);
  printf("is joint duplex = %d\n", val);

  val = snd_pcm_hw_params_can_overrange(params);
  printf("can overrange = %d\n", val);

  val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
  printf("can mmap = %d\n", val);

  val = snd_pcm_hw_params_can_pause(params);
  printf("can pause = %d\n", val);

  val = snd_pcm_hw_params_can_resume(params);
  printf("can resume = %d\n", val);

  val = snd_pcm_hw_params_can_sync_start(params);
  printf("can sync start = %d\n", val);

  snd_pcm_close(handle);

  return 0;
}


Listing 2 opens the default PCM device, sets some parameters and then displays the values of most of the hardware parameters. It does not perform any sound playback or recording. The call to snd_pcm_open opens the default PCM device and sets the access mode to PLAYBACK. This function returns a handle in the first function argument that is used in subsequent calls to manipulate the PCM stream. Like most ALSA library calls, the function returns an integer return status, a negative value indicating an error condition. In this case, we check the return code; if it indicates failure, we display the error message using the snd_strerror function and exit. In the interest of clarity, I have omitted most of the error checking from the example programs. In a production application, one should check the return code of every API call and provide appropriate error handling.

In order to set the hardware parameters for the stream, we need to allocate a variable of type snd_pcm_hw_params_t. We do this with the macro snd_pcm_hw_params_alloca. Next, we initialize the variable using the function snd_pcm_hw_params_any, passing the previously opened PCM stream.

We now set the desired hardware parameters using API calls that take the PCM stream handle, the hardware parameters structure and the parameter value. We set the stream to interleaved mode, 16-bit sample size, 2 channels and a 44,100 bps sampling rate. In the case of the sampling rate, sound hardware is not always able to support every sampling rate exactly. We use the function snd_pcm_hw_params_set_rate_near to request the nearest supported sampling rate to the requested value. The hardware parameters are not actually made active until we call the function snd_pcm_hw_params.

The rest of the program obtains and displays a number of the PCM stream parameters, including the period and buffer sizes. The results displayed vary somewhat depending on the sound hardware.

After running the program on your system, experiment and make some changes. Change the device name from default to hw:0,0 or plughw: and see whether the results change. Change the hardware parameter values and observe how the displayed results change.

Listing 3. Simple Sound Playback



/*

This example reads standard from input and writes
to the default PCM device for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

int main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
  frames = 32;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  size = frames * 4; /* 2 bytes/sample, 2 channels */
  buffer = (char *) malloc(size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
  loops = 5000000 / val;

  while (loops > 0) {
    loops--;
    rc = read(0, buffer, size);
    if (rc == 0) {
      fprintf(stderr, "end of file on input\n");
      break;
    } else if (rc != size) {
      fprintf(stderr,
              "short read: read %d bytes\n", rc);
    }
    rc = snd_pcm_writei(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}



Listing 3 extends the previous example by writing sound samples to the sound card to produce playback. In this case we read bytes from standard input, enough for one period, and write them to the sound card until five seconds of data has been transferred.

The beginning of the program is the same as in the previous example—the PCM device is opened and the hardware parameters are set. We use the period size chosen by ALSA and make this the size of our buffer for storing samples. We then find out that period time so we can calculate how many periods the program should process in order to run for five seconds.

In the loop that manages data, we read from standard input and fill our buffer with one period of samples. We check for and handle errors resulting from reaching the end of file or reading a different number of bytes from what was expected.

To send data to the PCM device, we use the snd_pcm_writei call. It operates much like the kernel write system call, except that the size is specified in frames. We check the return code for a number of error conditions. A return code of EPIPE indicates that underrun occurred, which causes the PCM stream to go into the XRUN state and stop processing data. The standard method to recover from this state is to use the snd_pcm_prepare function call to put the stream in the PREPARED state so it can start again the next time we write data to the stream. If we receive a different error result, we display the error code and continue. Finally, if the number of frames written is not what was expected, we display an error message.

The program loops until five seconds' worth of frames has been transferred or end of file read occurs on the input. We then call snd_pcm_drain to allow any pending sound samples to be transferred, then close the stream. We free the dynamically allocated buffer and exit.

We should see that the program is not useful unless the input is redirected to something other than a console. Try running it with the device /dev/urandom, which produces random data, like this:


./example3 < /dev/urandom

The random data should produce white noise for five seconds.

Next, try redirecting the input to /dev/null or /dev/zero and compare the results. Change some parameters, such as the sampling rate and data format, and see how it affects the results.

Listing 4. Simple Sound Recording



/*

This example reads from the default PCM device
and writes to standard output for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

int main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;

  /* Open PCM device for recording (capture). */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
  frames = 32;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
  size = frames * 4; /* 2 bytes/sample, 2 channels */
  buffer = (char *) malloc(size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);
  loops = 5000000 / val;

  while (loops > 0) {
    loops--;
    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc));
    } else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }
    rc = write(1, buffer, size);
    if (rc != size)
      fprintf(stderr,
              "short write: wrote %d bytes\n", rc);
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}


Listing 4 is much like Listing 3, except that we perform PCM capture (recording). When we open the PCM stream, we specify the mode as SND_PCM_STREAM_CAPTURE. In the main processing loop, we read the samples from the sound hardware using snd_pcm_readi and write it to standard output using write. We check for overrun and handle it in the same manner as we did underrun in Listing 3.

Running Listing 4 records approximately five seconds of data and sends it to standard out; you should redirect it to a file. If you have a microphone connected to your sound card, use a mixer program to set the recording source and level. Alternatively, you can run a CD player program and set the recording source to CD. Try running Listing 4 and redirecting the output to a file. You then can run Listing 3 to play back the data:


./listing4 > sound.raw
./listing3 < sound.raw

If your sound card supports full duplex sound, you should be able to pipe the programs together and hear the recorded sound coming out of the sound card by typing: ./listing4 | ./listing3. By changing the PCM parameters you can experiment with the effect of sampling rates and formats.

Advanced Features
In the previous examples, the PCM streams were operating in blocking mode, that is, the calls would not return until the data had been transferred. In an interactive event-driven application, this situation could lock up the application for unacceptably long periods of time. ALSA allows opening a stream in nonblocking mode where the read and write functions return immediately. If data transfers are pending and the calls cannot be processed, ALSA returns an error code of EBUSY.

Many graphical applications use callbacks to handle events. ALSA supports opening a PCM stream in asynchronous mode. This allows registering a callback function to be called when a period of sample data has been transferred.

The snd_pcm_readi and snd_pcm_writei calls used here are similar to the Linux read and write system calls. The letter i indicates that the frames are interleaved; corresponding functions exist for non-interleaved mode. Many devices under Linux also support the mmap system call, which maps them into memory where they can be manipulated with pointers. Finally, ALSA supports opening a PCM channel in mmap mode, which allows efficient zero copy access to sound data.

Conclusion
I hope this article has motivated you to try some ALSA programming. As the 2.6 kernel becomes commonly used by Linux distributions, ALSA should become more widely used, and its advanced features should help Linux audio applications move forward.

My thanks to Jaroslav Kysela and Takashi Iwai for reviewing a draft of this article and providing me with useful comments.

Resources for this article: /article/7705.

Jeff Tranter has been using, writing about and contributing to Linux since 1992. He works for Xandros Corporation in Ottawa, Canada.