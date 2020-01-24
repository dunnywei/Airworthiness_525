->python run shell command and get output to variable
https://stackoverflow.com/questions/20004307/run-a-complex-grep-command-with-python
https://cmdlinetips.com/2014/03/how-to-run-a-shell-command-from-python-and-get-the-output/
https://cmdlinetips.com/2014/03/how-to-run-a-shell-command-from-python-and-get-the-output/

->read/write access with pthread in C
https://stackoverflow.com/questions/55730428/reader-writer-problem-in-c-using-pthreads

->for C test
/*couting number of 1's in 0000 0001b
0000 000d &0000 0001==1
0000 00d0 &0000 0010==2
*/

void counting(unsigned char input)
{
	int counter=0;
	for(int a=0;i<8;i++)
	{
		if(input&(1<<i)==i)
		{
			counter++;
		}
	}
}