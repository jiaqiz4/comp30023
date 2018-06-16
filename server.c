/* A simple server in the internet domain using TCP
The port number is passed as an argument 


 To compile: gcc server.c -o server 
*/



//jiaqiz4
//num: 743304

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "sha256.h"
#include "uint256.h"
#include <assert.h>
#include <memory.h>
#include <time.h>
#include <arpa/inet.h>


void *connection_handler(void *socket_desc);
int checksolnformat(char buffer[]);
int checkworkformat(char buffer[]);
void write_to_log(char* string);

FILE* logfp;
const static char* perf_log = "log.txt";
pthread_mutex_t lock;

int main(int argc, char **argv)
{
    int sockfd, *newsockfd, portno, clilen,clientsocket;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    remove(perf_log);

    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

     /* Create TCP socket */
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }

    
    bzero((char *) &serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    
    /* Create address we're going to listen on (given port number)
     - converted to network byte order & any IP address for 
     this machine */
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);  // store in machine-neutral format

     /* Bind address to the socket */
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
        exit(1);
    }
    
    /* Listen on socket - means we're ready to accept connections - 
     incoming connection requests will be queued */
    
    listen(sockfd,100);   
    //clilen = sizeof(cli_addr);

    /* Accept a connection - block until a connection is ready to
     be accepted. Get back a new file descriptor to communicate on. */

    //pthread_t this_thread;
    while(clientsocket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)){
        newsockfd=malloc(1);
        *newsockfd = clientsocket;
        clilen = sizeof(cli_addr);
        pthread_t this_thread;


        time_t ltime; /* calendar time */
        ltime=time(NULL);

        //write into log
        write_to_log(asctime( localtime(&ltime) ) );
        write_to_log("client adress: ");
        write_to_log(inet_ntoa(cli_addr.sin_addr));
        write_to_log("\n");
        write_to_log("file descriptor: ");
        char id[100];
        sprintf(id,"%d",clientsocket);
        write_to_log(id);
        write_to_log("\n");
        

        //create a thread
        if( pthread_create( &this_thread, NULL , connection_handler, (void*)newsockfd) < 0){
                perror("could not create thread");
                exit(1);
        }

    }

    if (newsockfd < 0) {
        perror("accept failed");
        exit(1);
    }

    close(sockfd);
    
    return 0; 
}



void *connection_handler(void *socket_desc) {

    // Get socket descriptor
    int newsockfd = *(int*)socket_desc; 
    int n;
    char buffer[256];
    bzero(buffer,256);

    while (n = read(newsockfd,buffer,255)){
      
        
        //bzero(buffer,256);

        

        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }


        //PING message
        if(strcmp(buffer,"PING\r\n")==0){
            write_to_log("message:PING\nreturn:PONG\n\n");
            n = write(newsockfd,"PONG\r\n",6);
            
        }

        //Pong message
        else if(strcmp(buffer,"PONG\r\n")==0){
            write_to_log("message:PONG\nreturn:ERRO\n\n");
            n = write(newsockfd,"ERRO\r\n",6);
        }

        //OKAY message
        else if(strcmp(buffer,"OKAY\r\n")==0){
            write_to_log("message:OKAY\neturn:ERRO\n\n");
            n = write(newsockfd,"ERRO\r\n",6);
        }

        //SOLN message
        else if(strncasecmp(buffer,"SOLN",4)==0){
            write_to_log("sent: ");
            write_to_log(buffer);
            if(checksolnformat(buffer)!=1){
                write_to_log("ERRO:wrongformat\n");
                n = write(newsockfd,"ERRO:wrongformat\r\n",18);


            }

            int i=5;
            int j=0;
            char difficulty[9];
            char seed[65];
            char solution[17];


            //separate strings
            for(i=5;i<=12;i++){
                difficulty[j]=buffer[i];
                j++;
            }
            difficulty[8]='\0';
            

            j=0;
            for(i=14;i<=77;i++){
                seed[j]=buffer[i];
                j++;
            }
            seed[64]='\0';
            

            j=0;
            for(i=79;i<=94;i++){
                solution[j]=buffer[i];
                j++;
            }
            solution[16]='\0';


            //combine seed and solution
            char combine[81];
            j=0;
            while(j<64){
                combine[j]=seed[j];

                j++;
            }
            i=0;
            while(j<80){
                combine[j]=solution[i];
                i++;
                j++;

            }

            combine[80]='\0';
             //printf("%s\n",solution);

            BYTE x[40];
            char *pos=combine;
            size_t count;

 

            //change from 80 bits char to 40 bits BYTE
            for(count = 0; count < 40; count++) {
                sscanf(pos, "%2hhx", &x[count]);
                pos += 2;

            }

        

            //do hsh
            BYTE y[32];

            SHA256_CTX ctx;

            sha256_init(&ctx);
            sha256_update(&ctx, x, 40);
            sha256_final(&ctx,y);

            

            sha256_init(&ctx);
            sha256_update(&ctx, y, 32);
            sha256_final(&ctx,y);

           

            //calculate the target
            char alph[3];
            char beta[7];
            alph[0]=difficulty[0];
            alph[1]=difficulty[1];
            alph[2]='\0';

            j=0;
            
            for(j=0;j<6;j++){
                 beta[j]=difficulty[j+2];
            }
            beta[6]='\0';

            BYTE betabyte[3];
            char *pos1=beta;

            for(count = 0; count < sizeof(betabyte)/sizeof(betabyte[0]); count++) {
                sscanf(pos1, "%2hhx", &betabyte[count]);
                pos1 += 2;
            }

            BYTE mybeta[32];
            uint256_init32(mybeta);

            mybeta[31]=betabyte[2];
            mybeta[30]=betabyte[1];
            mybeta[29]=betabyte[0];


            int myalph=0;

            sscanf(alph, "%x", &myalph);

            BYTE target[32];
            uint256_init32(target);

            uint256_sl (target, mybeta, 8*(myalph-3));

            
            //combine the target with y
            if(sha256_compare(target,y)==1){
                write_to_log("return:OKAY\n\n");
                n = write(newsockfd,"OKAY\r\n",6);

            }else{
                //error message
                write_to_log("return:ERRO\n\n");
                n = write(newsockfd,"ERRO\r\n",6);
            };
        }

//Stage 3 WORK message

        else if(strncasecmp(buffer,"WORK",4)==0){
            write_to_log("sent:");
            write_to_log(buffer);
            
            //check format
            if(checkworkformat(buffer)!=1){
                write_to_log("ERRO:wrongformat\n");
                n = write(newsockfd,"ERRO:wrongformat\r\n",18);
            }


            int i=5;
            int j=0;
            char difficulty[9];
            char seed[65];
            char start[17];


            //separate input strings
            for(i=5;i<=12;i++){
                difficulty[j]=buffer[i];
                j++;
            }
            difficulty[8]='\0';
            

            j=0;
            for(i=14;i<=77;i++){
                seed[j]=buffer[i];
                j++;
            }
            seed[64]='\0';
            

            j=0;
            for(i=79;i<=94;i++){
                start[j]=buffer[i];
                j++;
            }
            start[16]='\0';

            //convert start to 8 bits byte
            size_t count;
            BYTE startB[8];
            uint256_init8(startB);
            char *pos1=start;

            for(count = 0; count < 8; count++) {
                sscanf(pos1, "%2hhx", &startB[count]);
                pos1 += 2;

            }


            //convert seed to 32 bits byte
            BYTE seedB[32];
            uint256_init32(seedB);
            char *pos2=seed;

            for(count = 0; count < 32; count++) {
                sscanf(pos2, "%2hhx", &seedB[count]);
                pos2 += 2;

            }

            //combine seed ang start to make a 40 BYTE x
            BYTE x[40];
            uint256_init(x);
            
            for (i=0;i<32;i++){
                x[i]=seedB[i];
            }
            j=0;
            for(i=32;i<40;i++){
                x[i]=startB[j];
                j++;
            }

        
            //calculate target
            char alph[3];
            char beta[7];
            alph[0]=difficulty[0];
            alph[1]=difficulty[1];
            alph[2]='\0';

            j=0;
            
            for(j=0;j<6;j++){
                 beta[j]=difficulty[j+2];
            }
            beta[6]='\0';

            BYTE betabyte[3];
            char *pos0=beta;

            for(count = 0; count <3; count++) {
                sscanf(pos0, "%2hhx", &betabyte[count]);
                pos0+= 2;
            }

            BYTE mybeta[32];
            uint256_init32 (mybeta);

            mybeta[31]=betabyte[2];
            mybeta[30]=betabyte[1];
            mybeta[29]=betabyte[0];

          

            int myalph=0;

            sscanf(alph, "%x", &myalph);
           

            BYTE target[32];
            uint256_init32(target);

            uint256_sl (target, mybeta, 8*(myalph-3));


            // do hash, initialize y
            BYTE y[32];

            SHA256_CTX ctx;

            sha256_init(&ctx);
            sha256_update(&ctx, x, 40);
            sha256_final(&ctx,y);


            sha256_init(&ctx);
            sha256_update(&ctx, y, 32);
            sha256_final(&ctx,y);


            //convert one and nonce into BYTE
            BYTE one[32];
            uint256_init32(one);
            one[31] = 0x1;

            BYTE nonce[32];
            uint256_init32(nonce);
            j=0;
            for(i=24;i<32;i++){
                nonce[i]=startB[j];
                j++;
            }
            
            //compare y and target untial y is smaller than target
            int k=0;
            BYTE newx[41];
            while(sha256_compare(target,y)!=1){


                uint256_add (nonce,nonce,one);
            
                for (i=0;i<32;i++){
                    newx[i]=seedB[i];
                }
                j=24;
                for(i=32;i<40;i++){
                    newx[i]=nonce[j];
                    j++;
                }

                newx[40]='\0';

                SHA256_CTX ctx;

                sha256_init(&ctx);
                sha256_update(&ctx, newx, 40);
                sha256_final(&ctx,y);


                sha256_init(&ctx);
                sha256_update(&ctx, y, 32);
                sha256_final(&ctx,y);



            }

            //change the solution from the byte to string
            BYTE output[9];
            k=32;
            for(j=0;j<8;j++){
                output[j]=newx[k];
                k++;
            }
            output[8]='\0';

            char once[3];
            char myout[17];

            j=0;
            for (i=0;i<8;i++){
                sprintf(once, "%02x", output[i]);
                myout[j]=once[0];
                myout[j+1]=once[1];
                j=j+2;

            }
            myout[16]='\0';

           //change the buffer which is the final output
            j=0;
            i=0;
            char head[4]="SOLN";
            for(j=0;j<4;j++){
                buffer[j]=head[i];
                i++;
            }

            i=0;
            for(j=79;j<95;j++){
                buffer[j]=myout[i];
                i++;
            }
            
            buffer[j]='\r';
            buffer[j+1]='\n';
            buffer[j+2]='\0';

            write_to_log("return:");
            write_to_log(buffer);
            write_to_log("\n");
            n = write(newsockfd,buffer,100);

        }

        bzero(buffer,256);



        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }


    }


    return 0;
} 



//check the soln format
int checksolnformat(char buffer[]){
    if (buffer[4]==' ' && buffer[13]==' ' && buffer[78]==' ' &&buffer[95]=='\r'){
        return 1;
    }
    return 0;

}

//check the work format
int checkworkformat(char buffer[]){
    if (buffer[4]==' ' && buffer[13]==' ' && buffer[78]==' ' &&buffer[95]==' '&& buffer[98]=='\r'){
        return 1;
    }
    return 0;

}


//put things in logfile
//this small function is from online code
void write_to_log(char* string){
    pthread_mutex_lock(&lock);  
    logfp = fopen(perf_log,"a+");

    fprintf(logfp,"%s", string);
    
    fclose(logfp);
    pthread_mutex_unlock(&lock);
}



    


