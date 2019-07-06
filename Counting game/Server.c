#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
const char* writeFile = "write.db";
const char* writeFile1 ="write1.db";
int main(int argc, char** argv) {
		
	int fd;
	int result = 0; //결과값을 저장할 변수
	int* input;    //입력 변수 포인터
	sem_t* mysem;  //클라이언트와 공유할 세마포어 변수
	int value=10;	//semvalue를 저장할 변수
	sem_unlink("mysem"); //이전에 mysem이라는세마포어가있다면 삭제
	
	if((mysem = sem_open("mysem",O_CREAT,0777,2))==NULL) {
		perror("Sem Open Error");
		return 1;
	} //mysem 세마포어 open

	if((fd=open(writeFile,O_CREAT|O_RDWR, S_IRUSR|S_IWUSR))==-1) {
		perror("Open Error\n");
		return 1;
	} //공유할 writeFile 메모리 생성

	write(fd,"",sizeof(int)*100); //쓰기를 실행(int배열100칸만큼)

	input = mmap(NULL,sizeof(int)*100,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
		//공유메모리 설정
	input[0] = 0; 


	while(value!=1) {sem_getvalue(mysem,&value); } //프로세스1개들어올때까지
	puts("client1 hi");
	while(value!=0) {       
		sem_getvalue(mysem,&value);   } 
	//서버 2개가 들어올때까지 무한루프
	puts("client2 hi"); //프로세스2 들어오면 시작

	printf("===============================\n");
	printf("          Game Start !!! \n");
	printf("===============================\n");

	//참고사항 : client1의 PID는 input[99]에 저장되고,
	//	     client2의 PID는 input[98p에 저장된다.

	sem_wait(mysem); //첫번째 입력값 기다린다.
	result += input[0] ;// 첫번째값 더함
	printf("[%d]%d\n",input[98],input[0]);	//출력

	while(1) {	
		sem_wait(mysem);//wait 걸고	
		
		//'=' 입력시  반복문 종료
		
			 if(input[1]==1){
		       	printf("[%d]%c" , input[99],'+');
			result += input[0];
		
	 		 }
		//'+' 입력시 더하기
			else if(input[1]==2){
			       	printf("[%d]%c", input[99], '-');
				result -= input[0];
			}
			else if(input[1]==3){
			       	printf("[%d]%c", input[99], '=');
		//'=' 입력시 반복문 빠져나온다.(연산입력 종료)	
				input[97] =result;//input97에 결과저장
				break; 
			} //잘못된 연산자 입력시 경고메시지 출력
				else( puts("연산자가 잘못 입력되었습니다."));
		      printf("[%d]%d\n",input[98],input[0]);
			
	}	
	printf("\nresult = %d\n",result);
	sem_wait(mysem); //클라이언트가 정답을 입력하기를 기다린다.
	//이타이밍엔 클라이언트 정답입력값이  input[96]에 저장된다.
	if(input[96] == result) { //클라이언트2의 정답입력이 맞다면
		printf("player %d win! \n", input[98]); //클라이언트2승리
		input[95] = 1; //input[95]값을 승자 flag처럼 사용한다.
		//승자flag가 1이라면 클라이언트2의 승리!
	} else{ printf("player %d win! \n", input[99]);
		input[95] = 0 ;
	}	//답이 틀렸다면 승자 flag를 0으로 설정하고, 클라이언트1의 승리!
	
	sem_post(mysem);
       	//승자계산 끝났으니 정지중인 클라이언트를 실행시킨다.

	close(fd);		
}
