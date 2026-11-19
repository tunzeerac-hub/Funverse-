#include <graphics.h>
#include <conio.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const int W=1280, H=800;
const double PI=3.14159265358979323846;
int frameNo=0, pageNo=0, speedValue=2, ticketsSold=0;
int timeOfDay=0; // 0=day, 1=evening, 2=night
int season=0;    // 0=summer, 1=winter, 2=rainy
bool paused=false, helpOpen=false, lightsOn=true;

int playerX=640, playerY=680;
int playerDX=0, playerDY=0;
int playerDir=3;

const int MAXV=8;
const int MAXKIDS=6;

struct Visitor { int x; int y; int state; int shirt; int pants; int wait; int ride; };
Visitor people[MAXV];

struct Kid { int x,y,vx,vy; int shirt; int mode; int t; };
Kid kids[MAXKIDS];

struct Particle { int x,y,vy,vx; int life; };
const int MAXP=120;
Particle snow[MAXP];
Particle rainDrops[MAXP];

int rgb(int r,int g,int b){ return COLOR(r,g,b); }
void rectFill(int x1,int y1,int x2,int y2,int col){ setcolor(col); setfillstyle(SOLID_FILL,col); bar(x1,y1,x2,y2); }
void dot(int x,int y,int rx,int ry,int col){ setcolor(col); setfillstyle(SOLID_FILL,col); fillellipse(x,y,rx,ry); }
void thickLine(int x1,int y1,int x2,int y2,int col,int t=2){ setcolor(col); setlinestyle(SOLID_LINE,0,t); line(x1,y1,x2,y2); setlinestyle(SOLID_LINE,0,1); }
void txt(int x,int y,const char *s,int col=WHITE,int size=1,int bg=-1){
    setcolor(col);
    if(bg!=-1) setbkcolor(bg);
    settextstyle(DEFAULT_FONT,HORIZ_DIR,size);
    outtextxy(x,y,(char*)s);
}
void roundBox(int x1,int y1,int x2,int y2,int col){
    setcolor(col); setfillstyle(SOLID_FILL,col);
    bar(x1+12,y1,x2-12,y2); bar(x1,y1+12,x2,y2-12);
    fillellipse(x1+12,y1+12,12,12); fillellipse(x2-12,y1+12,12,12);
    fillellipse(x1+12,y2-12,12,12); fillellipse(x2-12,y2-12,12,12);
}

void initVisitors(){
    int shirts[MAXV]={rgb(235,90,110),rgb(65,155,220),rgb(255,175,65),rgb(160,100,205),rgb(70,180,130),rgb(245,130,65),rgb(90,120,220),rgb(220,90,160)};
    int pants[MAXV]={rgb(45,55,85),rgb(55,65,75),rgb(80,55,110),rgb(45,70,60),rgb(70,65,55),rgb(45,55,90),rgb(70,50,60),rgb(55,70,90)};
    for(int i=0;i<MAXV;i++){
        people[i].x=75-i*43; people[i].y=610; people[i].state=0;
        people[i].shirt=shirts[i]; people[i].pants=pants[i]; people[i].wait=i*30; people[i].ride=i%3;
    }
}

void initKids(){
    int shirts[MAXKIDS]={rgb(255,120,180),rgb(120,220,120),rgb(255,220,100),rgb(150,180,255),rgb(255,160,100),rgb(200,140,255)};
    for(int i=0;i<MAXKIDS;i++){
        kids[i].x=300+(i*140)%900;
        kids[i].y=680+(i%3)*30;
        kids[i].vx=(i%2)?2:-2;
        kids[i].vy=(i%3==0)?1:-1;
        kids[i].shirt=shirts[i];
        kids[i].mode=i%3;
        kids[i].t=i*20;
    }
}

void initParticles(){
    for(int i=0;i<MAXP;i++){
        snow[i].x=rand()%W; snow[i].y=rand()%H;
        snow[i].vy=1+rand()%3; snow[i].vx=(rand()%3)-1;
        snow[i].life=1;
        rainDrops[i].x=rand()%W; rainDrops[i].y=rand()%H;
        rainDrops[i].vy=8+rand()%6; rainDrops[i].vx=-2; rainDrops[i].life=1;
    }
}

void human(int x,int y,int shirt,int pants,int phase,bool child=false){
    int head=child?8:10, body=child?19:25, sw=(phase%2==0)?6:-6;
    dot(x,y+body+18,12,3,rgb(70,100,72));
    thickLine(x-3,y+body-1,x-8+sw,y+body+16,pants,4);
    thickLine(x+3,y+body-1,x+8-sw,y+body+16,pants,4);
    thickLine(x-8+sw,y+body+16,x-1+sw,y+body+18,rgb(40,35,35),4);
    thickLine(x+8-sw,y+body+16,x+15-sw,y+body+18,rgb(40,35,35),4);
    thickLine(x-6,y+8,x-13-sw/2,y+body-1,rgb(238,185,145),4);
    thickLine(x+6,y+8,x+13+sw/2,y+body-1,rgb(238,185,145),4);
    dot(x,y+body/2,9,body/2,shirt);
    rectFill(x-3,y-2,x+3,y+7,rgb(238,185,145));
    dot(x,y-head,head,head,rgb(238,185,145));
    dot(x,y-head-5,head,6,rgb(65,40,28));
    dot(x-head+2,y-head-1,4,7,rgb(65,40,28));
    dot(x-3,y-head,1,2,rgb(35,35,35)); dot(x+3,y-head,1,2,rgb(35,35,35));
    arc(x,y-head+4,205,335,3);
}

void player(int x,int y,int phase,int dir){
    int body=26, sw=(phase%2==0)?7:-7;
    int shirt=rgb(60,140,230), pants=rgb(40,50,80);
    dot(x,y+body+18,14,4,rgb(60,90,62));
    thickLine(x-3,y+body-1,x-9+sw,y+body+16,pants,5);
    thickLine(x+3,y+body-1,x+9-sw,y+body+16,pants,5);
    thickLine(x-9+sw,y+body+16,x-2+sw,y+body+18,rgb(30,25,25),5);
    thickLine(x+9-sw,y+body+16,x+16-sw,y+body+18,rgb(30,25,25),5);
    thickLine(x-7,y+8,x-15-sw/2,y+body-1,rgb(238,185,145),5);
    thickLine(x+7,y+8,x+15+sw/2,y+body-1,rgb(238,185,145),5);
    dot(x,y+body/2,10,body/2,shirt);
    rectFill(x-3,y-2,x+3,y+7,rgb(238,185,145));
    dot(x,y-10,10,10,rgb(238,185,145));
    dot(x,y-15,10,6,rgb(50,30,20));
    dot(x-8,y-11,4,7,rgb(50,30,20));
    if(dir==0){ dot(x-3,y-10,1,2,rgb(20,20,20)); dot(x+3,y-10,1,2,rgb(20,20,20)); }
    else if(dir==1){ dot(x-4,y-10,2,2,rgb(20,20,20)); }
    else if(dir==2){ dot(x+4,y-10,2,2,rgb(20,20,20)); }
    else { dot(x-2,y-10,1,2,rgb(20,20,20)); dot(x+2,y-10,1,2,rgb(20,20,20)); }
    arc(x,y-6,205,335,3);
    dot(x,y-20,11,4,rgb(220,50,60));
    rectFill(x-11,y-20,x+11,y-17,rgb(220,50,60));
}

void cloud(int x,int y){
    int c;
    if(timeOfDay==2) c=rgb(70,85,115);
    else if(timeOfDay==1) c=rgb(255,180,140);
    else c=rgb(250,253,255);
    dot(x,y,23,14,c); dot(x+23,y-10,23,20,c); dot(x+48,y,25,15,c);
    rectFill(x-1,y,x+50,y+10,c);
}

void tree(int x,int y){
    rectFill(x-7,y-2,x+7,y+48,rgb(115,72,42));
    int c1,c2,c3;
    if(season==1){ c1=rgb(220,235,245); c2=rgb(200,220,240); c3=rgb(180,210,235); }
    else if(season==2){ c1=rgb(40,110,70); c2=rgb(50,125,80); c3=rgb(35,95,60); }
    else { c1=rgb(39,139,75); c2=rgb(52,158,83); c3=rgb(29,120,67); }
    dot(x,y-23,27,29,c1); dot(x-18,y-9,20,21,c2); dot(x+18,y-9,20,21,c3);
    dot(x-7,y-36,13,13,c1);
}

void drawSky(){
    int sky;
    if(timeOfDay==2) sky=rgb(15,26,59);
    else if(timeOfDay==1) sky=rgb(230,130,90);
    else sky=rgb(112,197,240);
    rectFill(0,0,W,H,sky);

    if(timeOfDay==2){
        dot(1090,92,39,39,rgb(255,232,160));
        dot(1107,78,34,36,sky);
        for(int i=0;i<55;i++){
            int x=(i*113+37)%W, y=(i*47+21)%310;
            dot(x,y,1+(i%2),1+(i%2),rgb(255,250,215));
        }
    } else if(timeOfDay==1){
        dot(1090,180,55,55,rgb(255,140,60));
        dot(1090,180,45,45,rgb(255,200,90));
        cloud((frameNo/2)%1450-100,105);
        cloud(410-(frameNo/4)%500,160);
    } else {
        dot(1090,92,43,43,rgb(255,220,75));
        cloud((frameNo/2)%1450-100,105);
        cloud(410-(frameNo/4)%500,160);
        cloud(850+(frameNo/5)%500,115);
    }

    int hR, hG, hB, lR, lG, lB, pR, pG, pB, peR, peG, peB;
    if(season==1){
        hR=220; hG=235; hB=245; lR=235; lG=245; lB=250; pR=200; pG=215; pB=225; peR=230; peG=240; peB=250;
    }
    else if(season==2){
        hR=55; hG=120; hB=85; lR=50; lG=130; lB=70; pR=140; pG=125; pB=105; peR=165; peG=150; peB=125;
    }
    else {
        hR=78; hG=158; hB=116; lR=65; lG=151; lB=83; pR=218; pG=192; pB=151; peR=246; peG=226; peB=190;
    }

    if(timeOfDay==2){
        hR/=2; hG/=2; hB/=2; lR/=2; lG/=2; lB/=2; pR/=2; pG/=2; pB/=2; peR/=2; peG/=2; peB/=2;
    }
    else if(timeOfDay==1){
        hR=(int)(hR*0.8); hG=(int)(hG*0.8); hB=(int)(hB*0.8); lR=(int)(lR*0.8); lG=(int)(lG*0.8); lB=(int)(lB*0.8); pR=(int)(pR*0.9); pG=(int)(pG*0.9); pB=(int)(pB*0.9); peR=(int)(peR*0.9); peG=(int)(peG*0.9); peB=(int)(peB*0.9);
    }

    int hillC = rgb(hR, hG, hB);
    int lawnC = rgb(lR, lG, lB);
    int pathC = rgb(pR, pG, pB);
    int pathEdge = rgb(peR, peG, peB);

    setcolor(hillC); setfillstyle(SOLID_FILL,hillC);
    fillellipse(210,345,290,85); fillellipse(700,340,360,90); fillellipse(1130,350,260,80);
    rectFill(0,350,W,H,lawnC);

    rectFill(0,625,W,718,pathC);
    thickLine(0,625,W,625,pathEdge,3);
    thickLine(0,718,W,718,pathEdge,3);

    rectFill(390,225,670,345,rgb(165,43,57));
    rectFill(405,239,655,330,rgb(255,218,103));
    setcolor(WHITE); rectangle(390,225,670,345);
    txt(440,270,"FUNVERSE",rgb(66,52,79),4,rgb(255,218,103));
    txt(450,315,"MAGIC STARTS HERE",rgb(120,70,45),1,rgb(255,218,103));
    tree(310,440); tree(365,455); tree(850,450); tree(1190,445);
}

void drawTicketBooth(){
    rectFill(28,420,190,590,rgb(244,171,91));
    rectFill(18,400,200,435,rgb(174,48,61));
    rectFill(47,450,171,487,rgb(255,244,211));
    txt(68,462,"TICKETS",rgb(95,55,42),1,rgb(255,244,211));
    rectFill(58,505,160,568,rgb(111,178,207));
    txt(82,529,"OPEN",WHITE,1,rgb(111,178,207));
    human(110,486,rgb(54,135,185),rgb(48,56,90),frameNo/8,true);
    txt(42,580,"TICKET COUNTER",WHITE,1);
}

void drawQueue(){
    for(int x=220;x<=390;x+=55){
        rectFill(x-3,510,x+3,614,rgb(205,190,155));
        dot(x,510,7,7,rgb(218,190,135));
    }
    thickLine(220,535,390,535,rgb(185,40,60),3);
    thickLine(220,610,390,610,rgb(185,40,60),3);
    txt(220,485,"TICKET QUEUE",WHITE,1);
    for(int i=0;i<MAXV;i++){
        if(people[i].state==0){
            int x=360-i*24;
            human(x,535,people[i].shirt,people[i].pants,frameNo/8,i%3==0);
        }
    }
    if(frameNo%260<170) {
        human(205,535,rgb(242,116,89),rgb(55,65,90),frameNo/7,false);
        if(frameNo%260==0) ticketsSold++;
        txt(198,570,"BUYING",rgb(255,245,210),1);
    }
}

void drawEntryFlow(){
    rectFill(690,500,704,625,rgb(115,125,137));
    rectFill(704,520,755,530,rgb(240,202,75));
    txt(680,480,"ENTRY GATE",WHITE,1);
    dot(710,495,7,7,(frameNo%40<20)?rgb(70,240,120):rgb(40,120,65));
    for(int i=0;i<MAXV;i++){
        int x=770+((frameNo*2+i*112)%470);
        int y=638+(i%2)*38;
        human(x,y,people[i].shirt,people[i].pants,frameNo/6,i%3==0);
    }
    txt(790,740,"WELCOME! ENJOY YOUR DAY",WHITE,1);
}

void drawFerrisWheel(int cx,int cy,int r){
    thickLine(cx,cy+15,cx-58,cy+r+90,rgb(100,105,119),9);
    thickLine(cx,cy+15,cx+58,cy+r+90,rgb(100,105,119),9);
    thickLine(cx-58,cy+r+90,cx+58,cy+r+90,rgb(100,105,119),7);
    setcolor(rgb(245,248,255)); circle(cx,cy,r); circle(cx,cy,r-5);
    dot(cx,cy,11,11,rgb(255,198,58));
    for(int i=0;i<10;i++){
        double a=frameNo*0.018+i*2*PI/10;
        int x=cx+(int)(r*cos(a)), y=cy+(int)(r*sin(a));
        thickLine(cx,cy,x,y,rgb(215,229,242),2);
        int col=(i%3==0)?rgb(247,90,112):((i%3==1)?rgb(255,205,72):rgb(72,185,220));
        roundBox(x-13,y-11,x+13,y+13,col);
        dot(x,y-2,4,4,rgb(238,185,145));
    }
    txt(cx-72,cy+r+100,"FERRIS WHEEL",WHITE,1);
}

void drawCarousel(int cx,int cy){
    setcolor(WHITE); ellipse(cx,cy+55,0,360,82,18);
    rectFill(cx-5,cy-5,cx+5,cy+60,rgb(210,210,220));
    setcolor(rgb(255,220,95)); setfillstyle(SOLID_FILL,rgb(230,76,135)); pieslice(cx,cy,0,360,70);
    circle(cx,cy,70);
    for(int i=0;i<8;i++){
        double a=frameNo*0.025+i*2*PI/8;
        int x=cx+(int)(48*cos(a)), y=cy+(int)(48*sin(a));
        thickLine(x,y-15,x,y+13,WHITE,2);
        dot(x,y,9,13,(i%2)?rgb(255,219,80):rgb(74,190,220));
        dot(x,y-4,3,3,rgb(238,185,145));
    }
    txt(cx-35,cy+88,"CAROUSEL",WHITE,1);
}

void drawCoaster(){
    int xs[]={760,800,845,890,935,980,1025,1070,1115,1160};
    int ys[]={465,420,385,400,455,490,450,385,405,465};
    for(int i=0;i<9;i++){
        thickLine(xs[i],ys[i],xs[i+1],ys[i+1],rgb(255,214,80),7);
        thickLine(xs[i],ys[i]+8,xs[i+1],ys[i+1]+8,rgb(220,235,245),2);
    }
    for(int i=0;i<9;i+=2) thickLine(xs[i],ys[i]+7,xs[i],550,rgb(120,95,70),4);
    int t=frameNo%360;
    for(int c=0;c<3;c++){
        int idx=(t/40+c*2)%9;
        int sub=t%40;
        int x=xs[idx]+(xs[idx+1]-xs[idx])*sub/40;
        int y=ys[idx]+(ys[idx+1]-ys[idx])*sub/40;
        roundBox(x-21,y-22,x+21,y+4,(c%2)?rgb(65,145,220):rgb(232,75,75));
        for(int p=-1;p<=1;p++){
            dot(x+p*12,y-26,5,5,rgb(238,185,145));
            thickLine(x+p*12,y-21,x+p*12,y-10,rgb(35,45,75),3);
        }
        dot(x-13,y+5,5,5,rgb(35,35,45)); dot(x+13,y+5,5,5,rgb(35,35,45));
    }
    txt(900,565,"ROLLER COASTER",WHITE,1);
}

void drawFountain(){
    setcolor(rgb(130,220,235)); setfillstyle(SOLID_FILL,rgb(55,151,190)); fillellipse(540,575,100,29);
    setcolor(rgb(200,245,255)); ellipse(540,575,0,360,100,29);
    rectFill(525,535,555,576,rgb(205,205,215));
    dot(540,535,18,7,rgb(225,225,235));
    for(int i=0;i<9;i++){
        int x=500+i*10;
        int h=25+(int)(18*(0.5+0.5*sin(frameNo*0.08+i)));
        thickLine(540,535,x,535-h,rgb(125,225,255),2);
        dot(x,535-h,3,4,rgb(195,245,255));
    }
    txt(505,615,"MAGIC FOUNTAIN",WHITE,1);
}

void drawFairyLights(){
    int posts[]={235,420,610,820,1010,1200};
    for(int i=0;i<5;i++){
        thickLine(posts[i],360,posts[i+1],360,rgb(65,70,75),1);
        for(int j=0;j<7;j++){
            int x=posts[i]+(posts[i+1]-posts[i])*j/7;
            int y=360+12*(j%2);
            int phase=(frameNo/8+j+i)%3;
            int col;
            if(timeOfDay==0) col=rgb(255,225,120);
            else if(timeOfDay==1) col=((phase==0)?rgb(255,180,80):((phase==1)?rgb(255,100,80):rgb(255,220,100)));
            else col=((phase==0)?rgb(255,80,130):((phase==1)?rgb(100,230,255):rgb(255,220,75)));
            dot(x,y,(timeOfDay==2&&lightsOn)?6:3,(timeOfDay==2&&lightsOn)?6:3,col);
            if(timeOfDay==2&&lightsOn && phase==0){ setcolor(rgb(255,150,190)); circle(x,y,10); }
        }
    }
    if(timeOfDay==2&&lightsOn){
        for(int x=270;x<1200;x+=230){
            rectFill(x-3,445,x+3,625,rgb(65,65,75));
            dot(x,440,10,10,rgb(255,232,140));
            setcolor(rgb(255,220,130)); circle(x,440,16);
        }
    }
}

void drawFlowers(){
    for(int x=230;x<380;x+=25){
        dot(x,600,4,4,rgb(255,100,145)); dot(x+7,600,3,3,rgb(255,220,80));
    }
}

void drawKids(){
    for(int i=0;i<MAXKIDS;i++){
        int x=kids[i].x;
        int y=kids[i].y;
        int phase=frameNo/5;
        if(kids[i].mode==0){
            x+=(int)(sin(frameNo*0.05+i)*30);
            y+=(int)(cos(frameNo*0.07+i)*10);
        } else if(kids[i].mode==1){
            y-=(int)(fabs(sin(frameNo*0.1+i))*15);
            phase=frameNo/3;
        } else {
            double a=frameNo*0.04+i;
            x+=(int)(cos(a)*40);
            y+=(int)(sin(a)*15);
        }
        human(x,y,kids[i].shirt,rgb(70,90,140),phase,true);
        if(i==0){
            int bx=x+15+(int)(sin(frameNo*0.3)*20);
            int by=y+5-(int)(fabs(cos(frameNo*0.3))*25);
            dot(bx,by,5,5,rgb(240,80,80));
        }
    }
    txt(555,760,"KIDS PLAYING AREA",WHITE,1);
}

void drawWeather(){
    if(season==1){
        for(int i=0;i<MAXP;i++){
            snow[i].y+=snow[i].vy;
            snow[i].x+=snow[i].vx;
            if(snow[i].y>H){ snow[i].y=0; snow[i].x=rand()%W; }
            if(snow[i].x<0) snow[i].x=W;
            if(snow[i].x>W) snow[i].x=0;
            dot(snow[i].x,snow[i].y,2,2,rgb(255,255,255));
        }
    } else if(season==2){
        for(int i=0;i<MAXP;i++){
            rainDrops[i].y+=rainDrops[i].vy;
            rainDrops[i].x+=rainDrops[i].vx;
            if(rainDrops[i].y>H){ rainDrops[i].y=0; rainDrops[i].x=rand()%W; }
            if(rainDrops[i].x<0) rainDrops[i].x=W;
            thickLine(rainDrops[i].x,rainDrops[i].y,rainDrops[i].x-3,rainDrops[i].y+10,rgb(160,200,245),1);
        }
    }
}

const char* getZoneMessage(){
    if(playerX>480&&playerX<640&&playerY>430&&playerY<560) return ">> FERRIS WHEEL ZONE: Enjoy the view! <<";
    if(playerX>690&&playerX<830&&playerY>430&&playerY<560) return ">> CAROUSEL ZONE: Ride the horses! <<";
    if(playerX>760&&playerX<1160&&playerY>350&&playerY<600) return ">> ROLLER COASTER ZONE: Scream time! <<";
    if(playerX>430&&playerX<650&&playerY>540&&playerY<620) return ">> MAGIC FOUNTAIN: Make a wish! <<";
    if(playerX>200&&playerX<400&&playerY>480&&playerY<620) return ">> TICKET QUEUE ZONE <<";
    if(playerX>200&&playerX<1000&&playerY>650&&playerY<740) return ">> WALKING PATH: Explore the park! <<";
    return "";
}

void drawHUD(){
    roundBox(15,12,440,78,rgb(24,43,69));
    txt(30,25,"FUNVERSE - THEME PARK",WHITE,2,rgb(24,43,69));
    const char* tod = (timeOfDay==0)?"Day":((timeOfDay==1)?"Evening":"Night");
    const char* sea = (season==0)?"Summer":((season==1)?"Winter":"Rainy");
    char s[200]; sprintf(s,"Visitors: %d  Tickets: %d  Speed: %d  %s  %s",MAXV,ticketsSold,speedValue,tod,sea);
    txt(30,52,s,rgb(220,235,255),1,rgb(24,43,69));

    const char* msg=getZoneMessage();
    if(strlen(msg)>0){
        roundBox(W/2-260,150,W/2+260,182,rgb(30,50,80));
        txt(W/2-245,158,msg,rgb(255,240,150),1,rgb(30,50,80));
    }

    if(helpOpen){
        roundBox(880,600,1270,790,rgb(24,43,69));
        txt(900,615,"CONTROLS",WHITE,1,rgb(24,43,69));
        txt(900,635,"Arrow Keys: Move player",rgb(200,240,255),1,rgb(24,43,69));
        txt(900,653,"SPACE Pause | D Day | E Evening",WHITE,1,rgb(24,43,69));
        txt(900,671,"N Night | S Season | T Ticket",WHITE,1,rgb(24,43,69));
        txt(900,689,"L Lights | +/- Speed | Q Quit",WHITE,1,rgb(24,43,69));
        txt(900,707,"H Toggle this help",rgb(255,220,120),1,rgb(24,43,69));
        txt(900,730,"Tip: Walk near rides to see zone info!",rgb(160,255,180),1,rgb(24,43,69));
    }
}

void drawWelcome(){
    rectFill(0,0,W,H,rgb(17,30,65));
    dot(640,190,80,80,rgb(255,211,82));
    txt(390,310,"FUNVERSE",rgb(255,226,105),6,rgb(17,30,65));
    txt(330,405,"A DAY OF RIDES, MAGIC AND ADVENTURE",WHITE,2,rgb(17,30,65));
    txt(430,510,"PRESS ENTER TO ENTER THE PARK",rgb(160,235,255),1,rgb(17,30,65));
    txt(535,545,"PRESS Q TO EXIT",WHITE,1,rgb(17,30,65));
}

int main(){
    initwindow(W,H,"FunVerse 2.0 - Explore The Theme Park");
    initVisitors();
    initKids();
    initParticles();
    drawWelcome();
    while(true){
        char k=getch();
        if(k==13) break;
        if(k=='q'||k=='Q'){ closegraph(); return 0; }
    }

    while(true){
        if(kbhit()){
            char k=getch();
            // Arrow key handling
            if(k==0 || k==224){
                k=getch();
                if(k==72){ playerDY=-5; playerDX=0; playerDir=3; } // Up
                if(k==80){ playerDY=5; playerDX=0; playerDir=0; }  // Down
                if(k==75){ playerDX=-5; playerDY=0; playerDir=1; } // Left
                if(k==77){ playerDX=5; playerDY=0; playerDir=2; }  // Right
            } else {
                // Normal keys
                if(k=='q'||k=='Q') break;
                if(k==' ') paused=!paused;
                if(k=='d'||k=='D') timeOfDay=0;
                if(k=='e'||k=='E') timeOfDay=1;
                if(k=='n'||k=='N') timeOfDay=2;
                if(k=='s'||k=='S') season=(season+1)%3;
                if(k=='h'||k=='H') helpOpen=!helpOpen;
                if(k=='l'||k=='L') lightsOn=!lightsOn;
                if((k=='+'||k=='=')&&speedValue<8) speedValue++;
                if(k=='-'&&speedValue>1) speedValue--;
                if(k=='t'||k=='T') ticketsSold++;
            }
        }
        if(!paused) frameNo+=speedValue;

        if(playerDX!=0 || playerDY!=0){
            playerX+=playerDX;
            playerY+=playerDY;
            if(playerX<40) playerX=40;
            if(playerX>W-40) playerX=W-40;
            if(playerY<370) playerY=370;
            if(playerY>H-40) playerY=H-40;
            playerDX=0; playerDY=0;
        }

        setactivepage(pageNo);
        setvisualpage(1-pageNo);
        drawSky();
        drawFerrisWheel(560,455,105);
        drawCarousel(760,460);
        drawCoaster();
        drawFountain();
        drawTicketBooth();
        drawQueue();
        drawEntryFlow();
        drawFairyLights();
        drawFlowers();
        drawKids();
        drawWeather();
        player(playerX,playerY,frameNo/6,playerDir);
        drawHUD();

        setvisualpage(pageNo);
        pageNo=1-pageNo;
        delay(30);
    }
    closegraph();
    return 0;
}


