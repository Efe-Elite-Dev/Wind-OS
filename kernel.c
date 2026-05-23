/*
 * Wind OS  -  kernel.c  v8.0 (Evrensel .EXE ve .DEB Yükleyici Alt Sistemi)
 */
#include "kernel.h"

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef int            i32;
typedef signed char    i8;
#define NULL ((void*)0)

static volatile u32 *FB = (u32*)0;
static u32 SW = 1024, SH = 768, SP = 1024;
static u32 back_buffer[1024 * 768];

static OS_State gST = STATE_DESKTOP;
static OS_State pST = STATE_DESKTOP;
static int DIRTY = 1;

/* TEMEL RENKLER */
#define CB   0xFFF4F4F6u
#define CW   0xFFFFFFFFu
#define CK   0xFF000000u
#define BG_BASE 0xFF1E2124u 
#define PAN_BG  0x99282B30u  
#define PAN_BD  0xFF424549u  
#define CBL     0xFF7289DAu  
#define CTXT    0xFFDCDDDEu  
#define CGY     0xFF99AAB5u  
#define CRD     0xFFED4245u  
#define CGN     0xFF57F287u  
#define COR     0xFFFEE75Cu  
/* YENİ ALT SİSTEM RENKLERİ */
#define WIN_BLUE 0xFF0078D7u /* EXE Yükleyici Rengi */
#define LIN_ORG  0xFFE95420u /* DEB Yükleyici Rengi (Ubuntu) */

static inline u8   inb (u16 p)       {u8  v;__asm__ volatile("inb  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outb(u16 p, u8 v) {__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static inline u32  inl (u16 p)       {u32 v;__asm__ volatile("inl  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outl(u16 p, u32 v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}

static void *mcpy(void *d,const void *s,u32 n){ u8*dp=(u8*)d;const u8*sp=(const u8*)s;while(n--)*dp++=*sp++;return d; }
static u32 klen(const char *s){u32 n=0;while(s[n])n++;return n;}
static void kcpy(char *d,const char *s){while(*s)*d++=*s++;*d=0;}
static void to_hex(u32 val, char* buf) {
    const char* hex = "0123456789ABCDEF"; buf[0]='0'; buf[1]='x';
    for(int i=7; i>=0; i--) { buf[2+i] = hex[val & 0xF]; val >>= 4; } buf[10] = 0;
}

static const u8 F8[128][8]={
 [' ']={0,0,0,0,0,0,0,0},['!']={0x18,0x3C,0x3C,0x18,0x18,0,0x18,0},['"']={0x36,0x36,0,0,0,0,0,0},['#']={0x36,0x7F,0x36,0x36,0x7F,0x36,0x36,0},
 ['$']={0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0},['%']={0x63,0x33,0x18,0x0C,0x66,0x63,0,0},['&']={0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0},['\'']={0x06,0x0C,0,0,0,0,0,0},
 ['(']={0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0},[')']={0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0},['*']={0x66,0x3C,0xFF,0x3C,0x66,0,0,0},['+']={0,0x0C,0x0C,0x3F,0x0C,0x0C,0,0},
 [',']={0,0,0,0,0,0x18,0x18,0x0C},['-']={0,0,0,0x3F,0,0,0,0},['.']={0,0,0,0,0,0x18,0x18,0},['/']={0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0},
 ['0']={0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0},['1']={0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0},['2']={0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0},['3']={0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0},
 ['4']={0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0},['5']={0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0},['6']={0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0},['7']={0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0},
 ['8']={0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0},['9']={0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0},[':']={0,0x18,0x18,0,0x18,0x18,0,0},[';']={0,0x18,0x18,0,0x18,0x18,0x0C,0},
 ['<']={0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0},['=']={0,0x3F,0,0,0x3F,0,0,0},['>']={0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0},['?']={0x1E,0x33,0x30,0x18,0x0C,0,0x0C,0},
 ['@']={0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0},['A']={0x0C,0x1E,0x33,0x3F,0x33,0x33,0x33,0},['B']={0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0},['C']={0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0},
 ['D']={0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0},['E']={0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0},['F']={0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0},['G']={0x3C,0x66,0x03,0x73,0x63,0x66,0x7C,0},
 ['H']={0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0},['I']={0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},['J']={0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0},['K']={0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0},
 ['L']={0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0},['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0},['N']={0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0},['O']={0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0},
 ['P']={0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0},['Q']={0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0},['R']={0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0},['S']={0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0},
 ['T']={0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0},['U']={0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0},['V']={0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0},['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0},
 ['X']={0,0x63,0x36,0x1C,0x1C,0x36,0x63,0},['Y']={0,0x33,0x33,0x33,0x3E,0x30,0x33,0x1E},['Z']={0,0x3F,0x19,0x0C,0x26,0x3F,0,0},['{']={0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0},
 ['|']={0x18,0x18,0x18,0,0x18,0x18,0x18,0},['}']={0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0},['~']={0x6E,0x3B,0,0,0,0,0,0},
};

static inline void pp(i32 x,i32 y,u32 c){ if((u32)x<SW&&(u32)y<SH) back_buffer[(u32)y*SP+(u32)x]=c; }
static void fr(i32 x,i32 y,i32 w,i32 h,u32 c){
    if(w<=0||h<=0) return;
    i32 x1=x<0?0:x, y1=y<0?0:y; i32 x2=x+w>(i32)SW?(i32)SW:x+w; i32 y2=y+h>(i32)SH?(i32)SH:y+h;
    for(i32 j=y1;j<y2;j++) for(i32 i=x1;i<x2;i++) back_buffer[(u32)j*SP+(u32)i]=c;
}
static void rb(i32 x,i32 y,i32 w,i32 h,u32 c,i32 t){ fr(x,y,w,t,c); fr(x,y+h-t,w,t,c); fr(x,y,t,h,c); fr(x+w-t,y,t,h,c); }
static void circ(i32 cx,i32 cy,i32 r,u32 c){
    if(r<=0) return;
    for(i32 dy=-r;dy<=r;dy++) for(i32 dx=-r;dx<=r;dx++) if(dx*dx+dy*dy<=r*r) pp(cx+dx,cy+dy,c);
}
static void rr(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c){
    if(r>w/2) r=w/2; if(r>h/2) r=h/2;
    fr(x+r,y,w-2*r,h,c); fr(x,y+r,r,h-2*r,c); fr(x+w-r,y+r,r,h-2*r,c);
    circ(x+r,y+r,r,c); circ(x+w-r-1,y+r,r,c); circ(x+r,y+h-r-1,r,c); circ(x+w-r-1,y+h-r-1,r,c);
}
static void dc(i32 x,i32 y,char ch,u32 fg,u32 bg,i32 sc){
    if((u8)ch>=128) ch='?'; const u8 *g=F8[(u8)ch];
    for(i32 row=0;row<8;row++) for(i32 col=0;col<8;col++) if(g[row]&(1<<(7-col))) fr(x+col*sc,y+row*sc,sc,sc,fg);
}
static void ds(i32 x,i32 y,const char*s,u32 fg,u32 bg,i32 sc){
    i32 cx=x; while(*s){ if(*s=='\n'){cx=x;y+=8*sc+2;} else{dc(cx,y,*s,fg,bg,sc);cx+=8*sc;} s++; }
}
static void dsc(i32 x,i32 y,i32 w,const char*s,u32 fg,u32 bg,i32 sc){
    i32 tw=(i32)klen(s)*8*sc; if(tw<w) ds(x+(w-tw)/2,y,s,fg,bg,sc); else ds(x,y,s,fg,bg,sc);
}
static void swap_buffers(void) { u32 total = SW * SH; for(u32 i = 0; i < total; i++) FB[i] = back_buffer[i]; }

/* KLAVYE VE MOUSE */
static char KBD_BUF[512] = "> Merhaba, kod yazmaya basla...\n"; static int KBD_LEN = 32;
static u8 K_SH=0, K_CP=0;
static const char SCMAP[128]={ 0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0,0,0,0,0,0,0,0,0 };

static u8 kbd_poll(void){
    u8 st=inb(0x64); if(!(st&0x01)) return 0;
    if( (st&0x20)){ inb(0x60); return 0; } 
    u8 sc=inb(0x60);
    if(sc&0x80){ u8 r=sc&0x7F; if(r==0x2A||r==0x36) K_SH=0; return 0; }
    if(sc==0x2A||sc==0x36){K_SH=1;return 0;} if(sc==0x3A){K_CP=!K_CP;return 0;} if(sc>=128) return 0;
    char c=SCMAP[sc]; if(!c) return 0;
    if(c>='a'&&c<='z'){ if(K_SH^K_CP) c-=32; } else if(K_SH){ switch(c){ case '1':c='!';break; case '2':c='@';break; case '3':c='#';break; case '4':c='$';break; case '5':c='%';break; case '6':c='^';break; case '7':c='&';break; case '8':c='*';break; case '9':c='(';break; case '0':c=')';break; case '-':c='_';break; case '=':c='+';break; case '[':c='{';break; case ']':c='}';break; case ';':c=':';break; case '\'':c='"';break;case ',':c='<';break; case '.':c='>';break; case '/':c='?';break; case '`':c='~';break; case '\\':c='|';break; } }
    if (c == 8) { if (KBD_LEN > 0) KBD_BUF[--KBD_LEN] = 0; } else { if (KBD_LEN < 510) { KBD_BUF[KBD_LEN++] = c; KBD_BUF[KBD_LEN] = 0; } }
    return (u8)c;
}

static i32 MX=512,MY=384,MLB=0,MRB=0,PMLB=0;
static u8  MCY=0; static i8 MBF[3]={0}; static int MOUSE_READY=0;
static void m_cmd_wait(void){u32 t=100000;while(t--&&(inb(0x64)&0x02));}
static void m_dat_wait(void){u32 t=100000;while(t--&&!(inb(0x64)&0x01));}
static void m_write(u8 v){m_cmd_wait();outb(0x64,0xD4);m_cmd_wait();outb(0x60,v);}
static u8   m_read (void){m_dat_wait();return inb(0x60);}

static void mouse_init(void){
    m_cmd_wait(); outb(0x64,0xA8); m_cmd_wait(); outb(0x64,0x20); m_dat_wait(); u8 cfg=inb(0x60);
    cfg|=0x02; cfg&=~0x20; m_cmd_wait(); outb(0x64,0x60); m_cmd_wait(); outb(0x60,cfg);
    m_write(0xFF); u8 ack=m_read(); u8 ok=m_read(); m_read();
    if(ack==0xFA && ok==0xAA){ m_write(0xF6); m_read(); m_write(0xF4); m_read(); MOUSE_READY=1; }
}

static void mouse_poll(void){
    if(!MOUSE_READY) return;
    for(int iter=0;iter<16;iter++){
        u8 st=inb(0x64); if(!(st&0x01)) break; 
        if(!(st&0x20)){ inb(0x60); continue; } u8 dat=inb(0x60);
        switch(MCY){
          case 0: if(!(dat&0x08)){MCY=0;break;} MBF[0]=(i8)dat; MCY=1; break;
          case 1: MBF[1]=(i8)dat; MCY=2; break;
          case 2: MBF[2]=(i8)dat; MCY=0;{
            i32 dx=(i32)MBF[1]; i32 dy=(i32)MBF[2];
            if(MBF[0]&0x10) dx|=(i32)0xFFFFFF00; if(MBF[0]&0x20) dy|=(i32)0xFFFFFF00; if(MBF[0]&0x40) dx=0; if(MBF[0]&0x80) dy=0;
            MX+=dx; MY-=dy; if(MX<0) MX=0; if(MY<0) MY=0; if(MX>=(i32)SW) MX=(i32)SW-1; if(MY>=(i32)SH) MY=(i32)SH-1;
            PMLB=MLB; MLB=(MBF[0]&0x01)?1:0; MRB=(MBF[0]&0x02)?1:0;
          } break;
        }
    }
}
static int CLK(i32 x,i32 y,i32 w,i32 h){ return MLB&&!PMLB&&MX>=x&&MX<x+w&&MY>=y&&MY<y+h; }
static int HOV(i32 x,i32 y,i32 w,i32 h){ return MX>=x&&MX<x+w&&MY>=y&&MY<y+h; }

/* PCI VE USB DONANIM */
static u32 pci_rd(u8 bus,u8 dev,u8 fn,u8 off){ outl(0xCF8,0x80000000u|((u32)bus<<16)|((u32)dev<<11)|((u32)fn<<8)|(off&0xFC)); return inl(0xCFC); }
static int USB_OK=0; static char USB_HWID[32]="ID: Bilinmiyor";
static void pci_scan(void){
    USB_OK = 0;
    for(int b=0;b<4;b++) for(int d=0;d<32;d++){
        u32 id=pci_rd(b,d,0,0); if((id&0xFFFF)==0xFFFF) continue;
        u32 cls=pci_rd(b,d,0,8); u8 cc=(u8)(cls>>24),sc2=(u8)(cls>>16);
        if(cc==0x0C&&sc2==0x03) { USB_OK=1; to_hex(id, USB_HWID); }
    }
}

static int BTN_C(i32 x, i32 y, i32 w, i32 h, const char* lbl, u32 c_icon, int is_active) {
    u32 bg = HOV(x,y,w,h) ? 0xFF36393Fu : PAN_BG; if (is_active) bg = 0xFF4F545Cu;
    rr(x, y, w, h, 8, bg); rb(x, y, w, h, PAN_BD, 1); circ(x + w/2, y + 25, 12, c_icon); dsc(x, y + 45, w, lbl, CTXT, bg, 1);
    return CLK(x,y,w,h);
}

static void CUR(void){
    static const u8 cur[16][12]={ {1,0},{1,1,0},{1,2,1,0},{1,2,2,1,0},{1,2,2,2,1,0},{1,2,2,2,2,1,0},{1,2,2,2,2,2,1,0},{1,2,2,2,2,2,2,1,0},{1,2,2,2,2,2,2,2,1,0},{1,2,2,2,2,1,1,1,1,1,0},{1,2,2,1,2,2,1,0},{1,2,1,0,1,2,2,1,0},{1,1,0,0,1,2,2,1,0},{0,0,0,0,0,1,2,2,1,0},{0,0,0,0,0,1,2,2,1,0},{0,0,0,0,0,0,1,1,0} };
    for(int r=0;r<16;r++) for(int c=0;c<12;c++){ i32 px=MX+c, py=MY+r; if((u32)px>=SW||(u32)py>=SH) continue; if(cur[r][c]==1) pp(px,py,CW); else if(cur[r][c]==2) pp(px,py,CK); }
}

/* UYGULAMALAR (Tarayıcı ve Terminal başlangıçta yüklü değil) */
typedef struct{char n[20];int inst;u32 col;} App;
static App AP[8]={
    {"Mesajlar",1,0xFF0078D4u},{"Terminal",0,0xFF4CAF50u}, /* Terminal Kurulmadı (.deb ile kurulacak) */
    {"Kamera",  1,0xFFE91E63u},{"Harita",  1,0xFFFF9800u},
    {"Hesap",   1,0xFF8B008Bu},{"Muzik",   1,0xFF00BCD4u},
    {"Tarayici",0,0xFF03A9F4u}, /* Tarayıcı Kurulmadı (.exe ile kurulacak) */
    {"Ayarlar", 1,0xFF9E9E9Eu},
};

static int FO=0, FU=0, TERM_OPEN=0;
static i32 FX=120,FY=90;
static int FD=0, FDX=0, FDY=0, TDrag=0, TDX=0, TDY=0, TX=400, TY=100;
static int FS=-1;

/* YÜKLEME MOTORU */
static int INSTALLING = 0; /* 0=Yok, 1=EXE(Windows), 2=DEB(Linux) */
static int INSTALL_PROG = 0;

static int is_ext(const char *n, const char *ext) {
    int nl = klen(n), el = klen(ext);
    if(nl <= el) return 0;
    for(int i=0;i<el;i++) if(n[nl-el+i] != ext[i]) return 0;
    return 1;
}

static void TERMINAL(void) {
    if(!TERM_OPEN) return; i32 TW=500, TH=350;
    if (!TDrag && MLB && !PMLB && MY >= TY && MY < TY + 30 && MX >= TX && MX < TX + TW) { TDrag = 1; TDX = MX - TX; TDY = MY - TY; }
    if (TDrag) { if (MLB) { TX = MX - TDX; TY = MY - TDY; if(TX<0)TX=0; if(TY<0)TY=0; if(TX>SW-TW)TX=SW-TW; if(TY>SH-TH)TY=SH-TH; } else TDrag = 0; }
    rr(TX, TY, TW, TH, 8, CK); rb(TX, TY, TW, TH, PAN_BD, 1);
    fr(TX, TY, TW, 30, PAN_BG); ds(TX+10, TY+10, "WIND TERMINAL", CW, 0, 1);
    if (BTN_C(TX + TW - 30, TY + 5, 25, 20, "", CRD, 0)) TERM_OPEN = 0;
    ds(TX+10, TY+40, KBD_BUF, CGN, 0, 1);
}

typedef struct{char n[32];int d;} FSE;
static FSE LFS[]={ {"Sistem",1},{"Belgeler",1},{"Indirmeler",1},{"Resimler",1},{"kernel.bin",0} };
/* YENİ: Hem EXE hem DEB dosyasını destekliyor! */
static FSE UFS[]={ {"ChromeSetup.exe",0},{"Araclar.deb",0} }; 

static void FILEMGR(void){
    if(!FO) return; i32 fw=600,fh=400,fx=FX,fy=FY;
    if(!FD&&MLB&&!PMLB&&MY>=fy&&MY<fy+36&&MX>=fx&&MX<fx+fw){FD=1;FDX=MX-fx;FDY=MY-fy;}
    if(FD){ if(MLB){ FX=MX-FDX; FY=MY-FDY; if(FX<0)FX=0; if(FY<0)FY=0; if(FX>(i32)SW-fw)FX=(i32)SW-fw; if(FY>(i32)SH-fh)FY=(i32)SH-fh; } else FD=0; }

    rr(fx,fy,fw,fh,10,BG_BASE); rb(fx,fy,fw,fh,PAN_BD,1);
    fr(fx,fy+30,fw,1,PAN_BD); ds(fx+15,fy+10,"DOSYA YONETICISI",CTXT,0,1);
    if(BTN_C(fx+fw-35,fy+5,30,20,"",CRD,0)) FO=0;

    i32 sb=140; fr(fx+sb,fy+31,1,fh-31,PAN_BD);
    const char*si[]={"Yerel Disk (C:)","Belgeler","Muzikler"};
    for(int i=0;i<3;i++) { if(CLK(fx,fy+40+i*30,sb,30)) FU=0; ds(fx+15,fy+50+i*30,si[i],!FU&&i==0?CW:CGY,0,1); }
    
    if(USB_OK){
        if(CLK(fx,fy+150,sb,30)) FU=1; circ(fx+20,fy+165,6,CBL); ds(fx+35,fy+161,"USB BAGLI",FU?CW:CBL,0,1);
        ds(fx+35,fy+175,USB_HWID,CGY,0,1);
    }

    i32 cx2=fx+sb+15, cy2=fy+45; FSE *en=FU?UFS:LFS; int cnt=FU?2:5;
    for(int i=0;i<cnt;i++){
        i32 ex=cx2+(i%4)*100, ey=cy2+(i/4)*90; if(ex+80>fx+fw || ey+80>fy+fh) continue;
        u32 bg = (FS==i) ? 0xFF4F545Cu : PAN_BG; rr(ex,ey,80,70,8,bg);
        
        int len=klen(en[i].n);
        int isExe = is_ext(en[i].n,".exe");
        int isDeb = is_ext(en[i].n,".deb");

        if(en[i].d){ fr(ex+25,ey+15,30,20,COR); } 
        else {
            fr(ex+30,ey+15,20,25,CW); 
            if(isExe) fr(ex+30,ey+30,20,10,WIN_BLUE); /* Mavi Windows Simgesi */
            else if(isDeb) circ(ex+40,ey+25,8,LIN_ORG); /* Turuncu Linux Simgesi */
        }

        char sn[12]={0}; if(len>10){mcpy(sn,en[i].n,8);sn[8]='.';sn[9]='.';} else kcpy(sn,en[i].n); dsc(ex,ey+50,80,sn,CTXT,0,1);
        
        /* DOSYAYA TIKLAMA OLAYI (ALT SİSTEMİ ÇAĞIRIR) */
        if(CLK(ex,ey,80,70)){
            FS=i;
            if(FU && !en[i].d && !INSTALLING) {
                if(isExe && !AP[6].inst) { INSTALLING = 1; INSTALL_PROG = 0; } /* EXE Kur */
                if(isDeb && !AP[1].inst) { INSTALLING = 2; INSTALL_PROG = 0; } /* DEB Kur */
            }
        }
    }
}

static void DESKTOP(void){
    fr(0, 0, (i32)SW, (i32)SH, BG_BASE);
    for(i32 i=0; i<SW; i+=40) fr(i, i, 1, SH, 0xFF23272Au); for(i32 i=0; i<SH; i+=40) fr(0, i, SW, 1, 0xFF23272Au);

    ds(60, 50, "UYGULAMALAR", CTXT, 0, 1);
    if(BTN_C(50, 100, 80, 80, "Ayarlar", AP[7].col, 0)) {}
    if(BTN_C(50, 220, 80, 80, "Kamera", AP[2].col, 0)) {}
    
    /* TERMINAL (Sadece .deb kurulduysa çalışır) */
    if(AP[1].inst) { if(BTN_C(50, 340, 80, 80, "Terminal", AP[1].col, TERM_OPEN)) TERM_OPEN=!TERM_OPEN; } 
    else { rr(50, 340, 80, 80, 8, 0xFF2F3136u); dsc(50, 380, 80, ".deb Gerek", 0xFF424549u, 0, 1); }

    ds(SW-180, 50, "SISTEM", CTXT, 0, 1);
    if(BTN_C(SW-160, 100, 80, 80, "Mesajlar", AP[0].col, 0)) {}
    if(BTN_C(SW-160, 220, 80, 80, "Dosyalar", COR, FO)) FO=!FO;
    if(BTN_C(SW-160, 340, 80, 80, "Muzik", AP[5].col, 0)) {}

    /* TARAYICI (Sadece .exe kurulduysa çalışır) */
    i32 gx = 250, gy = 150;
    if(AP[6].inst) { BTN_C(gx+110, gy, 80, 80, "Tarayici", AP[6].col, 0); } 
    else { rr(gx+110, gy, 80, 80, 8, 0xFF2F3136u); dsc(gx+110, gy+190, 80, ".exe Gerek", 0xFF424549u, 0, 1); }

    ds(300, SH-80, "WIND OS // CORE V8", CGY, 0, 1);
    if(USB_OK) { circ(500, SH-76, 5, CGN); ds(520, SH-80, "USB (DONANIM KIMLIGI OKUNDU)", CGN, 0, 1); }

    FILEMGR(); TERMINAL();
    
    /* ALT SISTEM YUKLEME EKRANI (EXE ve DEB İÇİN FARKLI) */
    if(INSTALLING) {
        i32 px = SW/2 - 170, py = SH/2 - 60;
        
        if(INSTALLING == 1) { /* WINDOWS EXE ALTYAPISI */
            rr(px-10, py-30, 360, 120, 10, WIN_BLUE);
            ds(px, py-20, "Windows Alt Sistemi (.EXE)", CW, 0, 1);
            ds(px, py, "ChromeSetup_v92.exe Calistiriliyor...", CW, 0, 1);
        } 
        else if (INSTALLING == 2) { /* LINUX DEB ALTYAPISI */
            rr(px-10, py-30, 360, 120, 10, LIN_ORG);
            ds(px, py-20, "Linux Alt Sistemi (.DEB)", CW, 0, 1);
            ds(px, py, "Araclar.deb Paket Aciliyor...", CW, 0, 1);
        }
        
        rr(px, py+40, 300, 20, 5, CK); /* Boş Bar */
        rr(px, py+40, INSTALL_PROG * 3, 20, 5, CW); /* Dolu Bar */
        
        INSTALL_PROG += 1;
        if(INSTALL_PROG >= 100) {
            if(INSTALLING == 1) AP[6].inst = 1; /* Tarayıcı simgesini aç */
            if(INSTALLING == 2) AP[1].inst = 1; /* Terminal simgesini aç */
            INSTALLING = 0;
        }
    }
    
    static int UTCK = 0; if(++UTCK>3000){pci_scan();UTCK=0;}
}

void kernel_main(multiboot_info_t *mbi){
    u8 bpp  = mbi->framebuffer_bpp; if(bpp==0) bpp=32; u32 Bpp = (u32)bpp / 8;
    FB  = (volatile u32*)(unsigned long)mbi->framebuffer_addr; SW  = mbi->framebuffer_width; SH  = mbi->framebuffer_height; SP  = mbi->framebuffer_pitch / Bpp;
    if(!FB || SW==0){ FB=(volatile u32*)0xFD000000u; SW=1024; SH=768; SP=1024; }

    mouse_init(); pci_scan(); gST = STATE_DESKTOP;

    while(1){
        mouse_poll(); kbd_poll(); if(gST!=pST){DIRTY=1;pST=gST;}
        switch(gST){ case STATE_DESKTOP: DESKTOP(); break; default: break; }
        CUR(); swap_buffers(); volatile int x=50000;while(x--)__asm__("nop");
    }
}
