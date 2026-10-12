#include<iostream>
#include<windows.h>
#include<cwchar>
#include<cmath>
#include<conio.h>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>

#define speed 0.2


using namespace std;
HANDLE console;
CONSOLE_FONT_INFOEX font{};
CONSOLE_SCREEN_BUFFER_INFOEX screen{};
//print element
SMALL_RECT area{
    0,0,599,199
};
COORD buf_size{600,200};
COORD start{0,0};


typedef struct elementinfo{
    int x,y;
    int type;
    string state="undefined";
}elementinfo;


CHAR_INFO buffer[200][600]{};
float buffer_dis[600];
DWORD written;//trash


vector<elementinfo> elements;
float player_x,player_y;
float sight=0;//0~360,11.25 도 씩 증가
BOOL ifw=false,ifa=false,ifd=false,ifs=false,ifleft=false,ifright=false,ifup=false;


int setTerminal();
void setPalette();
int mainmenu();
int mainplay(string room,int num);
void gotoxy(SHORT x,SHORT y){
    BOOL yar=SetConsoleCursorPosition(console, COORD{x, y});
    return;
}
void printlog(string title,string message){
    MessageBoxA(
    GetConsoleWindow(),
    message.c_str(),  // \n으로 줄바꿈
    title.c_str(),
    MB_OK | MB_ICONINFORMATION
);
}
int iftri(float px,float py,float x1,float y1,float x2,float y2,float x3,float y3){
    float a = (x1 - px) * (y2 - y1) - (x2 - x1) * (y1 - py);
    float b = (x2 - px) * (y3 - y2) - (x3 - x2) * (y2 - px);
    float c = (x3 - px) * (y1 - y3) - (x1 - x3) * (y3 - px);
    return (a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0);
}//code from https://hi-develop.tistory.com/20
int main(){
    //preprocess
    console=GetStdHandle(STD_OUTPUT_HANDLE);
    if( console==INVALID_HANDLE_VALUE || console==nullptr){
        cout<<"[log].console handle failure";
        return 1;
    }

    //menu
    mainmenu();
    //preprocess2
    setTerminal();
    setPalette();
    //game
    mainplay("start",0);
    
}
int mainplay(string room,int num){

    //background process+ 기본 유니코드 넣기;
    for(int i=0;i<200;i++){
        for(int j=0;j<600;j++){
            buffer[i][j].Char.UnicodeChar=L'\u2588';
            buffer[i][j].Attributes=12;
        }
    }
    for(int i=0;i<600;i++){
        buffer_dis[i]=0;
    }
    WriteConsoleOutputW(console,&buffer[0][0],buf_size,start,&area);
    
    //get elements
    string path=".\\map\\"+room+".txt";
    ifstream file(path);
    string roominfo[12];
    for(int i=1;i<=11;i++){
        getline(file,roominfo[i]);
    }
    string message;
    getline(file,message);
    if(message!="message undefined"){
        printlog("notification",message);
    }
    string n_string;
    getline(file,n_string);
    int element_n;
    element_n=stoi(n_string);
    for(int i=0;i<element_n;i++){   
        elements.push_back({0,0,0,"unheld"});
        string inputline;
        getline(file,inputline);
        istringstream elementline(inputline);
        elementline>>elements[element_n].x>>elements[element_n].y>>elements[element_n].type;
    }

    //set variables
    player_x=2;
    player_y=6;
    sight=0;
    //sight 은 맵 파일의 배열에서 플레이어를 원점으로 둔 그래프에서 계산

    //render screen
    //거리 계산 식
    //fov=60*
    //pixel의 각: |300-x|/5
    //최대 랜더 거리: 6 (시야)
    //
    //최대 랜더 거리부터 오브젝트 체크 및 버퍼에 그리기 반복->출력
    //히히 오줌발싸
    //랜더거리 계산: 한 오브당 약 3 색깔 사용
    //거리별로 -1, 거리 당 같은 색
    //렌더:8,
    //각 오브젝트의 크기: 
while(1){
    //event tick
    //remove input and prev screen
    ifw=false;
    ifa=false;
    ifd=false;
    ifs=false;
    ifleft=false;
    ifright=false;
    ifup=false;
    for(int i=0;i<600;i++){
        buffer_dis[i]=420.0f;
    }
    //get input
    if(GetAsyncKeyState('W') & 0x8000){
        ifw=true;
    }
    if(GetAsyncKeyState('A') & 0x8000){
        ifa=true;
    }
    if(GetAsyncKeyState('S') & 0x8000){
        ifs=true;
    }
    if(GetAsyncKeyState('D') & 0x8000){
        ifd=true;
    }
    if(GetAsyncKeyState(VK_UP) & 0x8000){
        ifup=true;
    }
    if(GetAsyncKeyState(VK_LEFT) & 0x8000){
        ifleft=true;
    }
    if(GetAsyncKeyState(VK_RIGHT) & 0x8000){
        ifright=true;
    }
    
    //apply movement
    if(ifleft){
        sight+=11.25;
    }
    if(ifright){
        sight-=11.25;
    }
    sight=fmod(sight,360);
    if(sight<0){
        sight+=360;
    }
    
    float sightrad=sight*3.14159265f/180.0f;

    float sins=sin(sightrad)*speed;
    float coss=cos(sightrad)*speed;
    float tans=tan(sightrad)*speed;
    if(ifw){
        player_x+=coss;
        player_y+=sins;
        if(roominfo[int(player_x)][int(player_y)]==1){
            player_x-=coss;
            player_y-=sins;
        }
    }
    if(ifa){
        player_x-=sins;
        player_y+=coss;
        if(roominfo[int(player_x)][int(player_y)]==1){
            player_x+=sins;
            player_y-=coss;
        }
    }
    if(ifs){
        player_x-=coss;
        player_y-=sins;
        if(roominfo[int(player_x)][int(player_y)]==1){
            player_x+=coss;
            player_y+=sins;
        }
    }
    if(ifd){
        player_x+=sins;
        player_y-=coss;
        if(roominfo[int(player_x)][int(player_y)]==1){
            player_x-=sins;
            player_y+=coss;
        }
    }
    //i love math!
    //apply element info
for(int i=0;i<element_n;i++){
if(elements[i].type==1){
        //to do
}else if(elements[i].type==2){
    if(elements[i].x==int(player_x) && elements[i].y==int(player_y)){
        elements[i].state="held";
        elements[i].x=-1;
        elements[i].y=-1;
    }
}
}
    //check flag
    if(int(player_x)==17 && int(player_y)==6){
        break;
    }

    //render screen(hardest)
    //how to render
    //explore the sight and draw obst
    
     //시야 범위 측정
    float leftsight=sight+60;
    float rightsight=sight-60;

    float lsrad=leftsight*3.14159265f/180.0f;
    float rsrad=rightsight*3.14159265f/180.0f;

    float cosls=cos(lsrad);
    float sinls=sin(lsrad);

    float cosrs=cos(rsrad);
    float sinrs=sin(rsrad);

    float ltarget_x=player_x+cosls*6;
    float ltarget_y=player_y+sinls*6;

    float rtarget_x=player_x+cosrs*6;
    float rtarget_y=player_y+sinrs*6;
    
    for(){

    }



    //add overlay


    //print screen
    WriteConsoleOutputW(console,&buffer[0][0],buf_size,start,&area);
}    
//end of event tick
    return 0;
}
int setTerminal(){
    system("cls");
    font.cbSize=sizeof(font);
    GetCurrentConsoleFontEx(console,FALSE,&font);

    font.dwFontSize.X=3;
    font.dwFontSize.Y=2;
    wcscpy(font.FaceName,L"Consolas");
    SetCurrentConsoleFontEx(console,FALSE,&font);
    
    COORD fullscreenSize{};
    SetConsoleDisplayMode(
    console,
    CONSOLE_FULLSCREEN_MODE,
    &fullscreenSize
    );

    COORD terminalsize{
        600,200
    };
    SMALL_RECT tWindow{0,0,0,0};
    SetConsoleWindowInfo(console, TRUE, &tWindow);
    SetConsoleScreenBufferSize(console,terminalsize);
    SetConsoleWindowInfo(console, TRUE, &area);
    return 0;   
}
void setPalette(){
    screen.cbSize=sizeof(screen);
    GetConsoleScreenBufferInfoEx(console,&screen);
    for(int i=0;i<13;i++){
        BYTE brightness=static_cast<BYTE>(i*255/13);
        screen.ColorTable[i]=RGB(brightness,brightness,brightness);
    }//12색
    screen.ColorTable[13]=RGB(255,0,0);//red.for enemy
    screen.ColorTable[14]=RGB(255,230,0);//yellow.for exit or key
    screen.ColorTable[15]=RGB(150,135,0);//dark yellow.for not-onsight key
    screen.srWindow.Right++;
    screen.srWindow.Bottom++;
    SetConsoleScreenBufferInfoEx(console,&screen);
    
}
int mainmenu(){
    cout.flush();
    system("cls");
    int cur=1;
    int input;
    cout<<"   terminal game 3d"<<"\n"
        <<"   play"<<"\n"
        <<"   settings"<<"\n"
        <<"   exit"<<"\n\n\n"
        <<"key: up down space";
    gotoxy(2,cur);
    cout<<">";
    while(1){
        input=1;
        input=_getch();
        if(input==0 || input==224){
            gotoxy(2,cur);
            cout<<" ";
            input=_getch();
            if(input==72){
                cur--;
                if(cur==0)cur=3;
            }
            if(input==80){
                cur++;
                if(cur==4)cur=1;
            }
            gotoxy(2,cur);
            cout<<">";
        }else if(input==32){
            if(cur==1){
                system("cls");
                cout<<"make screen 3:2\n";
                cout<<"using ctrl+scroll down\n";
                cout<<"press any key\n";
                input=_getch();
                return 0;
            }
            else if(cur==2)break;
            else{
                system("cls");
                exit(0);
            }
        }
    }
    //settings
    while(1){
        cout.flush();
        system("cls");
        cout<<"\n";
        cout<<"SETTINGS"<<"\n"
            <<"   exit"<<"\n";
        input=_getch();
        if(input==32){
            mainmenu();
            return 0;
        }
    }
}
//the end