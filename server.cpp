#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h>   
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>  
#define TRUE   1 
#define FALSE  0 
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

string boardstring(vector<vector<int>> v){
    string ans="";
    for(int i=0;i<v.size();i++){
        for(int j=0;j<v[i].size();j++){
            if(v[i][j]==0){
                ans+=" ";
            }
            else if(v[i][j]==1){
                ans+="O";
            }
            else{
                ans+="X";
            }
            if(j!=2){ans+="|";}
        }
        ans+="\n-----\n";
    }
    return ans;
}

bool is_correct_entry(vector<vector<int>> board,int r,int c){
    return board[r-1][c-1] == 0;
}

bool check_board(vector<vector<int>> board,int r,int c){
    r--;
    c--;
    bool flag=true;
    for(int i=0;i<3;i++){
        if(board[r][i]!=board[r][c]){flag=false;break;}
    }
    if(flag){return true;}
    flag=true;
    for(int i=0;i<3;i++){
        if(board[i][c]!=board[r][c]){flag=false;break;}
    }
    if(flag){return true;}
    if(r==c){
        flag=true;
        for(int i=0;i<3;i++){
            if(board[i][i]!=board[r][c]){flag=false;break;}
        }
        if(flag){return true;}
    }
    
    if(r==2-c){
        flag=true;
        for(int i=0;i<3;i++){
            if(board[r][c]!=board[i][2-i]){flag=false;break;}
        }
        if(flag){return true;}
    }
    return false;
}

bool check_draw(vector<vector<int>> board){
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            if(board[i][j]==0){
                return false;
            }
        }
    }
    return true;
}

int main(int argc , char *argv[])  
{  
    ofstream logfile;
    logfile.open("statistics.log");
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , 
          max_clients = 30 , activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
    char buffer[1025];   
    fd_set readfds;     
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }   
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
          
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
         
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
    addrlen = sizeof(address); 
    // important variables///////////////
    int id=1;
    map<int,int> socket_player_map;
    map<int,int> reverse_map;
    vector<vector<vector<int>>> boards;
    string ask_for_input="Enter (ROW, COL) for placing your mark:";
    map<int,int> partner_map;
    int waiting = -1;
    map<int,int> board_player_map;
    map<int,int> player_symbol_map;
    map<int,int> available_players;
    map<int,int> responses;
    map<int,struct timespec> start_times;
    map<int,int> gameids;
    int gameid=1;
    /////////////////////////////////////
    cout<<"Game server started. Waiting for players.\n";
    while(TRUE)  
    { 
        FD_ZERO(&readfds);  
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
        for ( i = 0 ; i < max_clients ; i++)  
        {
            sd = client_socket[i];   
            if(sd > 0)  
                FD_SET( sd , &readfds);   
            if(sd > max_sd)  
                max_sd = sd;  
        }  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }

        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }           
            for (i = 0; i < max_clients; i++)  
            {  
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    break;  
                }  
            } 
            // cout<<id<<"--"<<new_socket<<endl;
            socket_player_map[id]=new_socket;
            reverse_map[new_socket]=id;
            string s = "Connected to the game server. ";
            if(waiting==-1){
                waiting = id;
                s+="Your player ID is "+to_string(id)+" . Waiting for a partner to join . . .\n";
                send_message(new_socket,s);
            }
            else{
                gameids[id]=gameid;
                gameids[waiting]=gameid;
                clock_gettime(CLOCK_MONOTONIC,&start_times[id]);
                clock_gettime(CLOCK_MONOTONIC,&start_times[waiting]);
                time_t rawtime;
                struct tm * timeinfo;
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                logfile<<"match between player "<<id<<" and player "<<waiting<<" started at "<<asctime (timeinfo) <<endl;
                board_player_map[id]=boards.size();
                board_player_map[waiting]=boards.size();
                boards.push_back(vector<vector<int>>(3,vector<int>(3,0)));
                partner_map[id]=waiting;
                partner_map[waiting]=id;
                s+="Your player ID is "+to_string(id)+" . Your partner ID is " +to_string(waiting)+". Your symbol is X\n";
                player_symbol_map[id]=2;
                send_message(new_socket,s);
                int partner = partner_map[id];
                send_message(socket_player_map[id],"Starting the game\n");
                send_message(socket_player_map[id],boardstring(boards[board_player_map[id]]));
                send_message(socket_player_map[partner],"Your partner's ID is "+to_string(id)+". Your symbol is O.\n");
                player_symbol_map[partner]=1;
                send_message(socket_player_map[partner],"Starting the game\n");
                send_message(socket_player_map[partner],boardstring(boards[board_player_map[partner]]));
                // send_message(socket_player_map[id],ask_for_input);
                send_message(socket_player_map[partner],ask_for_input);
                waiting = -1;
                
                gameid++;
            }
            available_players[id]++;
            id++;
        }  
             
        for (i = 0; i < max_clients; i++)  
        {
            sd = client_socket[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                string response = read_message(sd);
                if (response == "empty")  
                {   
                    int player_id=reverse_map[sd];
                    available_players[player_id]--;
                    // cout<< "player "<<player_id<<" disconnected\n";
                    close( sd );  
                    client_socket[i] = 0;  
                    if(waiting == player_id){waiting = -1;}
                    else{
                        int partner = partner_map[player_id];
                        if(available_players[partner]==1){
                            // cout<<partner<<"-->"<<socket_player_map[partner]<<endl;
                            send_message(socket_player_map[partner],"$");
                            close(socket_player_map[partner]);
                            // cout<< "player "<<partner<<" disconnected\n";
                            available_players[partner]--;
                        }
                    }
                }    
                else{
                    int player = reverse_map[sd];
                    int partner = partner_map[player];
                    string rc = response;
                    // cout<<rc.size()<<" "<<rc <<endl;
                    if(rc.size()!=2 && rc!="y" && rc!="n" && rc!="$"){continue;}
                    if(rc=="n"){
                        send_message(sd,"$");
                        close(sd);
                        available_players[player]--;
                        if(available_players[partner]==1){
                            send_message(socket_player_map[partner],"$");
                            close(socket_player_map[partner]);
                            available_players[partner]--;
                        }
                        continue;
                    }
                    else if (rc=="y"){
                        // cout<<"reached"<<endl;
                        responses[player]=1;
                        // cout<<responses[player]<<' '<<responses[partner]<<endl;
                        if(responses[partner]==1){
                            boards[board_player_map[player]]=vector<vector<int>>(3,vector<int>(3,0));
                            responses[partner]=0;
                            responses[player]=0;
                            send_message(sd,boardstring(boards[board_player_map[player]]));
                            send_message(socket_player_map[partner],boardstring(boards[board_player_map[partner]]));
                            send_message(sd,ask_for_input);
                            gameids[player]=gameid;
                            gameids[partner]=gameid;
                            gameid++;
                        }
                        continue;
                    }
                    else if(rc=="$"){
                        send_message(sd,"do you want to play again?");
                        send_message(socket_player_map[partner],"do you want to play again?");
                        continue;
                    }
                    int r = rc[0]-'0';
                    int c = rc[1]-'0';
                    int temp = player_symbol_map[player];
                    vector<vector<int>> board = boards[board_player_map[player]];
                    if(!is_correct_entry(board,r,c)){
                        send_message(sd,"Incorrect input ! Try again\n");
                        send_message(sd,ask_for_input);
                    }
                    else{
                        struct timespec start ,end;
                        start=start_times[player];
                        clock_gettime(CLOCK_MONOTONIC,&end);
                        double temp_time = (end.tv_sec-start.tv_sec)*1000+(end.tv_nsec-start.tv_nsec)/1000000;
                        logfile<<"player "<<player<<" marked row , column = "<<r<<','<<c<<endl;
                        board[r-1][c-1]=temp;
                        // cout<<"reached"<<endl;
                        send_message(sd,boardstring(board));
                        send_message(socket_player_map[partner],boardstring(board));
                        // cout<<(boardstring(board));
                        if(check_board(board,r,c)){
                            logfile<<"player "<<player<<" won in game number "<<gameids[player]<<" and it took "<<temp_time/1000<<" seconds"<<endl;
                            send_message(sd,"wow player "+to_string(player)+" won !");
                            send_message(socket_player_map[partner],"oops player "+to_string(player)+" won !");
                            send_message(sd,"do you want to play again?");
                            send_message(socket_player_map[partner],"do you want to play again?");
                        }
                        else if(check_draw(board)){
                            logfile<<"game number "<<gameids[player]<<" is draw"<<" and it took "<<temp_time/1000<<" seconds"<<endl;
                            send_message(sd,"It's a draw !");
                            send_message(socket_player_map[partner],"It's a draw !");
                            send_message(sd,"do you want to play again (y/n)?");
                            send_message(socket_player_map[partner],"do you want to play again?");
                        }
                        else{ 
                            send_message(socket_player_map[partner],ask_for_input);
                        }
                    }
                    boards[board_player_map[player]] = board;
                    
                }
                 
            }  
        }  
    }  
         
    return 0;  
}  