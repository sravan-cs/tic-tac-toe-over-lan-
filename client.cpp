#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <bits/stdc++.h>
#define PORT 8080
using namespace std;

void send_message(int socket_id , string s ){
    char* message=&s[0];
    if( send(socket_id, message, strlen(message), 0) != strlen(message) )  
    {  
        perror("send");  
    }
}

string read_message(int socket_id){
    char buffer[1025];
    int valread;
    if ((valread = read( socket_id , buffer, 1024)) == 0){
        return "empty";
    }
    buffer[valread]=0;
    string s = buffer;
    return s;
}
 
int main(int argc, char const* argv[])
{
    string ask_for_input="Enter (ROW, COL) for placing your mark:";
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if (connect(sock, (struct sockaddr*)&serv_addr,
                sizeof(serv_addr))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while(true){
        struct timespec start ,end;
        string s;
        s=read_message(sock);
        if(s=="empty"){
            cout<<"server disconnected"<<endl;
            break;
        }
        if(s[s.size()-1]=='$'){
            cout<<"partner disconnected"<<endl;
            break;
        }
        cout<<s<<flush;
        if(s[s.size()-1]==':'){
            while(true){
                int a=1,b=1;
                clock_gettime(CLOCK_MONOTONIC,&start);
                cin>>a>>b;
                clock_gettime(CLOCK_MONOTONIC,&end);
                double temp_time = (end.tv_sec-start.tv_sec)*1000+(end.tv_nsec-start.tv_nsec)/1000000;
                if(temp_time>15e3){
                    cout<<"took too much time !"<<endl;
                    send_message(sock,"$");
                    break;
                }
                if(a>3 || b>3 || a<1 || b<1){cout<<"invalid input try again"<<endl;continue;}
                send_message(sock,to_string(a)+to_string(b));
                break;
            }
        }
        else if(s[s.size()-1]=='?'){
            string a ;
            cout<<endl<<"press y/n"<<endl;
            while(true){
                cin>>a;
                if(a=="y"||a=="Y"){
                    a="y";
                    break;
                }
                else if(a=="n"||a=="N"){
                    a="n";
                    break;
                }
                else{
                    cout<<"Invalid Input try again\n";
                }
            }
            // cout<<a<<endl;
            send_message(sock,a);
            
        }
    }
    
    return 0;
}