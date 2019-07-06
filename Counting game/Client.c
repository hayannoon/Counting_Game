#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const char* writeFile = "write.db";
const char* writeFile1 = "write1.db";
int main(int argc, char** argv) {
	int fd;
	int* input; //입력 변수
	pid_t PID; //PID 저장 변수
	sem_t* mysem; //서버와 공유하는 세마포어 변수
	sem_t* clientsem; //클라이언트끼리 공유하는 세마포어 변수1
	sem_t* clientsem1; //클라이언트끼리 공유하는 세마포어 변수2
	int flag = 0; //클라이언트1,2를 구별할 flag
	

	if((mysem = sem_open("mysem",0,0777,0))==SEM_FAILED) {
		perror("Sem Open Error");
		return 1;
	} //mysem 공유! 

	sem_getvalue(mysem,&flag);//flag값 저장(서버가 2명으로 차있을경우 예외
	if(flag == 0){
	      	puts("서버가 가득 차있습니다. 잠시후 다시 시도하세요.");
		return 0;
	} //이미 flag값이 0이라면 두개의 서버가 들어와있다는 의미이기때문에
	  //프로세스를 종료한다.
	//조건을 통과했다면
	sem_wait(mysem); //들어올수있는자리 하나 줄인다.
	sem_getvalue(mysem,&flag);//flag값 저장(먼저오면1 나중오면0)


	if(flag==1)  { //flag가 1이라면, 첫번째 클라이언트
		sem_unlink("clientsem");
	if((clientsem = sem_open("clientsem",O_CREAT,0777,0))==NULL) {
		perror("Sem Open Error");
		return 1;
	} //클라이언트끼리 공유할 세마포어 생성
		sem_unlink("clientsem1");	
	 if((clientsem1 = sem_open("clientsem1",O_CREAT,0777,0))==NULL) {
                perror("Sem Open Error");
                return 1;
        } //클라이언트끼리 공유할 두번째 세마포어 생성

	} //flag가 1인경우는 먼저들어온경우다. 
	//이때는 clientsem을 새로 만들어준다.
	

	else if(flag==0) { //flag가 0이라면 두번째 클라이언트
	if((clientsem = sem_open("clientsem",0,0777,0))==SEM_FAILED) {
		perror("Sem Open Error");
		return 1;
	} //flag가 0인경우는 두번째로 들어온 경우다.
	  //이때는 clientsem에 접근할수있도록 해준다.
	
	 if((clientsem1 = sem_open("clientsem1",0,0777,0))==SEM_FAILED) {
                perror("Sem Open Error");
                return 1;
        }

	}


	if((fd = open(writeFile,O_RDWR))==-1) {
		perror("Open Error\n");
		return 1;
	} //서버가 생성한 writeFile에 접근한다.



	write(fd, "", sizeof(int)*100);
	//쓰기를 허용한다.
	
	input = mmap(NULL,sizeof(int)*100,PROT_READ |PROT_WRITE, MAP_SHARED,fd,0);
	//input변수를 공유메모리로 사용한다.
	PID = getpid();
	//PID 저장
	
	if(flag==1) { //처음 진입한 프로세스는 두번째가 들어오길  기다린다.
	puts("=====Wait another client===="); 
		sem_wait(clientsem);//두번째 프로세스 기다린다.
		input[99] = PID;
 	//첫번째 프로세스의 PIDfmf input[99]에저장(서버와 공유하기위함)
		
	}
	else if(flag==0){
	       	sem_post(clientsem);//기다리던 클라이언트1 깨운다.
		input[98] = PID;
	} //두번째도 역시 PID값을 input[98]에 저장(서버와 공유하기위함)
	puts("============================");
	printf("        Hello %d         \n",PID);
	puts("        Game Start!!");
	puts("============================");
	sleep(2); //두번째프로세스가 들어오면 게임 시작!
//이시점에 clientsem, clientsem1 모두 value값 0이다.


	//최초 입력은 값을 무조건 더하기해야하기때문에, 최초입력만
	//따로 한번 실행한 뒤에 반복문으로 들어간다.
	int over100 = 1; //이 변수는 범위를 벗어난 입력을 받으면
			//재입력을 받기위해 사용하는 변수
	if(flag==0) {
		
		while(over100==1) {//정상적인 값이 들어오면 반복탈출
		printf("input num(1~100) : ");
		scanf("%d", &input[0]);
		if(! (input[0]>0 && input[0]<101)) {
		puts("숫자의 범위가 벗어났습니다. 다시 입력해주세요.");
		} else over100 = 0;}
	
		printf("[%d]%d \n",PID,input[0]); //입력받은거 저장
		sem_post(clientsem); //클라이언트2 실행시키고
		sem_wait(clientsem1); //클라이언트2가 서버 실행시킬때까지 대기
	} else {      
		sem_wait(clientsem);
		//클라이언트2는 클라이언트1이 입력때까지 대기
		sem_post(mysem);	//서버 재실행(입력끝났으므로)
		sem_post(clientsem1);//클라이언트1 깨워준다. 
	}
	
	while(1) { 
				
		if(flag==0) {  //클라이언트2 프로세스가 진입
		sem_wait(clientsem); //연산자입력하도록 기다린다
		if(input[1]==3) { // '=' 입력시 서버 깨워주고 반복문 탈출
			sem_post(mysem);
			 printf("[%d]%d(=)\n",input[99],input[1]);
			break;
		} 
		if(input[1]==1) printf("[%d]%d(+)\n",input[99],input[1]);
                else if(input[1]==2) printf("[%d]%d(-)\n",input[99],input[1]);
		//먼저 클라이언트1이 입력한 기호 출력해주고

		//위에처럼 범위에 알맞는 변수 입력받는다.
		 over100 =1;	          
		  while(over100==1) {
                printf("input num(1~100) : ");
                scanf("%d", &input[0]);
                if(! (input[0]>0 && input[0]<101)) {
                puts("숫자의 범위가 벗어났습니다. 다시 입력해주세요.");
                } else over100 = 0;}

		printf("[%d]%d \n",PID,input[0]);	
			sem_post(mysem);
			sem_post(clientsem1);
		} //올바른값 입력시 서버와 클라이언트1을 깨워준다.

	       
		else{ //클라이언트1  프로세스가 진입
		printf("[%d]%d\n",input[98],input[0]);
		printf("input +,-,= ( + : 1 , - : 2 , = : 3 ) : ");
		scanf("%d",&input[1]); //연산자 입력받는다.
		if(input[1]==1) printf("[%d]%d(+)\n",PID,input[1]);
		else if(input[1]==2) printf("[%d]%d(-)\n",PID,input[1]);
		else if(input[1]==3) { //입력받은 연산자정보 출력
		       	printf("[%d]%d(=)\n",PID,input[1]);
			sem_post(clientsem); 
			sleep(3);
			// '=' 입력시 클라이언트2 깨워주고 반복문 탈출한다.
			break;
		}

		sem_post(clientsem);//클라이언트2 깨워주고
		sem_wait(clientsem1);//기다린다
		}
	}

	//여기까지 왔다는건 '='를 입력하고 입력이 종료되었다는 뜻이다.
	//정답값을 클라이언트2로부터 입력받을것이다.
	
		if(flag==0) {
			printf("Calculate this operation :");
			scanf("%d",&input[96]),//input96에 결과값받는다.
			sem_post(mysem);//결과값 받은후 서버를 깨워준다.
			sleep(3);
			sem_wait(mysem);//서버에서 처리할 결과 기다린다.
			sem_post(clientsem);//만약 클라이언트1이 대기중이면깨운다.	
	       	} else {
			//다른 클라이언트도 결과가 나온뒤에 실행하기위함	
			sem_wait(clientsem);
			
			
		}
		puts("<3>");
		sleep(1);
		puts("<2>");
		sleep(1);
		puts("<1>");
		sleep(1);
		//결과 출력!
		//결과는 서버가 input[95]에 flag처럼 입력해놓았다.
		//input[95]가 1이라면 클라이언트2의 승리,
		//input[95]가 0이라면 클라이언트1의 승리로 약속한다.
		if(input[95]==1) {
			if(flag==0) puts("You Win!");
			else puts("You Lose!");
		} else{
			if(flag==0) puts("You Lose!");
			else puts("You Win!");
		} //서버가 전송한 결과에 따라 클라이언트 승리여부 출력
		
	close(fd);
}
