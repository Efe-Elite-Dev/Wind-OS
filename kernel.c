/*
 * Wind OS  -  kernel.c  v5 (.wind, .exe, .deb Destekli + Chrome Tarayıcı)
 * DÜZELTİLEN/EKLENENLER:
 * 1. wwind hatası düzeltildi, orijinal ".wind" uzantısı eklendi.
 * 2. Linux ".deb" paketi algılama ve çalıştırma eklendi.
 * 3. Dosya yöneticisinde uzantılara özel renkli etiketler ayarlandı.
 *
 * gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -O2 -w -c kernel.c -o kernel.o
 */
#include "kernel.h"

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef int            i32;
typedef signed char    i8;
#define NULL ((void*)0)

/* ── FRAMEBUFFER ─────────────────────────────────── */
static volatile u32 *FB = (u32*)0;
static u32 SW = 1024, SH = 768, SP = 1024;
static u32 back_buffer[1024 * 768];

/* ── DURUM ───────────────────────────────────────── */
static OS_State gST = STATE_DESKTOP;
static OS_State pST = STATE_DESKTOP;
static int DIRTY = 1;
static u32 SYS_TICK = 0;

/* ── RENKLER ─────────────────────────────────────── */
#define CB   0xFFF4F4F6u
#define CW   0xFFFFFFFFu
#define CK   0xFF000000u
#define CBL  0xFF0078D4u
#define CLL  0xFFDEECF9u
#define CGY  0xFF767676u
#define CLG  0xFFE5E5E5u
#define CMG  0xFFB4B4B4u
#define CDG  0xFF2D2D2Du
#define CRD  0xFFD13438u
#define CGN  0xFF107C10u
#define COR  0xFFFF8C00u
#define CBR  0xFFCCCCCCu
#define CTB  0xFF1C1C1Cu
#define CTH  0xFF383838u
#define CHD  0xFFEBEBEFu
#define C2   0xFF005A9Eu

/* ── PORT I/O ────────────────────────────────────── */
static inline u8   inb (u16 p)       {u8  v;__asm__ volatile("inb  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outb(u16 p, u8 v) {__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static inline u32  inl (u16 p)       {u32 v;__asm__ volatile("inl  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outl(u16 p, u32 v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}

/* ── YARDIMCILAR ─────────────────────────────────── */
static void *mcpy(void *d,const void *s,u32 n){
    u8*dp=(u8*)d;const u8*sp=(const u8*)s;while(n--)*dp++=*sp++;return d;
}
static u32 klen(const char *s){u32 n=0;while(s[n])n++;return n;}
static void kcpy(char *d,const char *s){while(*s)*d++=*s++;*d=0;}

static u32 isqrt(u32 n){
    if(!n)return 0; u32 x=n,y=1; while(x>y){x=(x+y)/2;y=n/x;} return x;
}

static void itoa(int n, char s[]){
    int i=0, sign;
    if ((sign = n) < 0) n = -n;
    do { s[i++] = n % 10 + '0'; } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--){ char temp = s[j]; s[j] = s[k]; s[k] = temp; }
}

/* ── 8x8 BITMAP FONT ─────────────────────────────── */
static const u8 F8[128][8]={
 [' ']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},['!']={0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
 ['"']={0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},['#']={0x36,0x7F,0x36,0x36,0x7F,0x36,0x36,0x00},
 ['$']={0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},['%']={0x63,0x33,0x18,0x0C,0x66,0x63,0x00,0x00},
 ['&']={0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},['\'']={0x06,0x0C,0x00,0x00,0x00,0x00,0x00,0x00},
 ['(']={0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},[')']={0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
 ['*']={0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00},['+']={0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
 [',']={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x0C},['-']={0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
 ['.']={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},['/']={0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
 ['0']={0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},['1']={0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
 ['2']={0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},['3']={0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
 ['4']={0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},['5']={0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
 ['6']={0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},['7']={0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
 ['8']={0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},['9']={0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
 [':']={0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},[';']={0x00,0x18,0x18,0x00,0x18,0x18,0x0C,0x00},
 ['<']={0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},['=']={0x00,0x3F,0x00,0x00,0x3F,0x00,0x00,0x00},
 ['>']={0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},['?']={0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
 ['@']={0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},['A']={0x0C,0x1E,0x33,0x3F,0x33,0x33,0x33,0x00},
 ['B']={0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},['C']={0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
 ['D']={0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},['E']={0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
 ['F']={0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},['G']={0x3C,0x66,0x03,0x73,0x63,0x66,0x7C,0x00},
 ['H']={0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},['I']={0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
 ['J']={0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},['K']={0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
 ['L']={0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
 ['N']={0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},['O']={0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
 ['P']={0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},['Q']={0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
 ['R']={0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},['S']={0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
 ['T']={0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},['U']={0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
 ['V']={0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
 ['X']={0x63,0x63,0x36,0x1C,0x36,0x63,0x63,0x00},['Y']={0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
 ['Z']={0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},['[']={0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00},
 ['\\']={0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},[']']={0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00},
 ['^']={0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00},['_']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
 ['`']={0x06,0x0C,0x18,0x00,0x00,0x00,0x00,0x00},['a']={0x00,0x1E,0x30,0x3E,0x33,0x33,0x6E,0x00},
 ['b']={0x07,0x06,0x3E,0x66,0x66,0x66,0x3B,0x00},['c']={0x00,0x1E,0x33,0x03,0x03,0x33,0x1E,0x00},
 ['d']={0x38,0x30,0x3E,0x33,0x33,0x33,0x6E,0x00},['e']={0x00,0x1E,0x33,0x3F,0x03,0x33,0x1E,0x00},
 ['f']={0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00},['g']={0x00,0x6E,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['h']={0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00},['i']={0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00},
 ['j']={0x18,0x00,0x18,0x18,0x18,0x1B,0x1B,0x0E},['k']={0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00},
 ['l']={0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},['m']={0x00,0x33,0x7F,0x7F,0x6B,0x63,0x63,0x00},
 ['n']={0x00,0x1F,0x33,0x33,0x33,0x33,0x33,0x00},['o']={0x00,0x1E,0x33,0x33,0x33,0x33,0x1E,0x00},
 ['p']={0x00,0x3B,0x66,0x66,0x3E,0x06,0x06,0x0F},['q']={0x00,0x6E,0x33,0x33,0x3E,0x30,0x30,0x78},
 ['r']={0x00,0x3B,0x6E,0x66,0x06,0x06,0x0F,0x00},['s']={0x00,0x3E,0x03,0x1E,0x30,0x33,0x1E,0x00},
 ['t']={0x08,0x3E,0x0C,0x0C,0x0C,0x2C,0x18,0x00},['u']={0x00,0x33,0x33,0x33,0x33,0x33,0x6E,0x00},
 ['v']={0x00,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},['w']={0x00,0x63,0x6B,0x7F,0x7F,0x36,0x36,0x00},
 ['x']={0x00,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},['y']={0x00,0x33,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['z']={0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00,0x00},['{']={0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00},
 ['|']={0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},['}']={0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00},
 ['~']={0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00},
};

/* ================================================================
   ÇİZİM MOTURU (BOUNDS-CHECKED + DOUBLE BUFFERING)
   ================================================================ */
static inline void pp(i32 x,i32 y,u32 c){
    if((u32)x<SW&&(u32)y<SH) back_buffer[(u32)y*SP+(u32)x]=c;
}
static void fr(i32 x,i32 y,i32 w,i32 h,u32 c){
    if(w<=0||h<=0) return;
    i32 x1=x<0?0:x, y1=y<0?0:y;
    i32 x2=x+w>(i32)SW?(i32)SW:x+w;
    i32 y2=y+h>(i32)SH?(i32)SH:y+h;
    for(i32 j=y1;j<y2;j++)
        for(i32 i=x1;i<x2;i++)
            back_buffer[(u32)j*SP+(u32)i]=c;
}
static void rb(i32 x,i32 y,i32 w,i32 h,u32 c,i32 t){
    fr(x,y,w,t,c); fr(x,y+h-t,w,t,c); fr(x,y,t,h,c); fr(x+w-t,y,t,h,c);
}
static void circ(i32 cx,i32 cy,i32 r,u32 c){
    if(r<=0) return;
    for(i32 dy=-r;dy<=r;dy++)
        for(i32 dx=-r;dx<=r;dx++)
            if(dx*dx+dy*dy<=r*r) pp(cx+dx,cy+dy,c);
}
static void rr(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c){
    if(r>w/2) r=w/2; if(r>h/2) r=h/2;
    fr(x+r,y,w-2*r,h,c); fr(x,y+r,r,h-2*r,c); fr(x+w-r,y+r,r,h-2*r,c);
    circ(x+r,y+r,r,c); circ(x+w-r-1,y+r,r,c); circ(x+r,y+h-r-1,r,c); circ(x+w-r-1,y+h-r-1,r,c);
}
static void dc(i32 x,i32 y,char ch,u32 fg,u32 bg,i32 sc){
    if((u8)ch>=128) ch='?'; const u8 *g=F8[(u8)ch];
    for(i32 row=0;row<8;row++)
        for(i32 col=0;col<8;col++)
            fr(x+col*sc,y+row*sc,sc,sc, (g[row]&(1<<(7-col)))?fg:bg);
}
static void ds(i32 x,i32 y,const char*s,u32 fg,u32 bg,i32 sc){
    i32 cx=x; while(*s){ if(*s=='\n'){cx=x;y+=8*sc;} else{dc(cx,y,*s,fg,bg,sc);cx+=8*sc;} s++; }
}
static void dsc(i32 x,i32 y,i32 w,const char*s,u32 fg,u32 bg,i32 sc){
    i32 tw=(i32)klen(s)*8*sc; if(tw<w) ds(x+(w-tw)/2,y,s,fg,bg,sc); else ds(x,y,s,fg,bg,sc);
}
static void swap_buffers(void) {
    u32 total = SW * SH; for(u32 i = 0; i < total; i++) FB[i] = back_buffer[i];
}

/* ================================================================
   SÜRÜCÜLER (KLAVYE, MOUSE, USB TARA)
   ================================================================ */
static u8 K_SH=0, K_CP=0;
static const char SCMAP[128]={ 0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0,0,0,0,0,0,0,0,0 };
static u8 kbd_poll(void){
    u8 st=inb(0x64); if(!(st&0x01)) return 0;
    if( (st&0x20)){ inb(0x60); return 0; } 
    u8 sc=inb(0x60);
    if(sc&0x80){ u8 r=sc&0x7F; if(r==0x2A||r==0x36) K_SH=0; return 0; }
    if(sc==0x2A||sc==0x36){K_SH=1;return 0;} if(sc==0x3A){K_CP=!K_CP;return 0;} if(sc>=128) return 0;
    char c=SCMAP[sc]; if(!c) return 0;
    if(c>='a'&&c<='z'){ if(K_SH^K_CP) c-=32; }
    else if(K_SH){
        switch(c){ case '1':c='!';break; case '2':c='@';break; case '3':c='#';break; case '4':c='$';break; case '5':c='%';break; case '6':c='^';break; case '7':c='&';break; case '8':c='*';break; case '9':c='(';break; case '0':c=')';break; case '-':c='_';break; case '=':c='+';break; case '[':c='{';break; case ']':c='}';break; case ';':c=':';break; case '\'':c='"';break;case ',':c='<';break; case '.':c='>';break; case '/':c='?';break; case '`':c='~';break; case '\\':c='|';break; }
    }
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
        if(!(st&0x20)){ inb(0x60); continue; }
        u8 dat=inb(0x60);
        switch(MCY){
          case 0: if(!(dat&0x08)){MCY=0;break;} MBF[0]=(i8)dat; MCY=1; break;
          case 1: MBF[1]=(i8)dat; MCY=2; break;
          case 2: MBF[2]=(i8)dat; MCY=0;{
            i32 dx=(i32)MBF[1]; i32 dy=(i32)MBF[2];
            if(MBF[0]&0x10) dx|=(i32)0xFFFFFF00; if(MBF[0]&0x20) dy|=(i32)0xFFFFFF00;
            if(MBF[0]&0x40) dx=0; if(MBF[0]&0x80) dy=0;
            MX+=dx; MY-=dy;
            if(MX<0)MX=0; if(MY<0)MY=0; if(MX>=(i32)SW) MX=(i32)SW-1; if(MY>=(i32)SH) MY=(i32)SH-1;
            PMLB=MLB; MLB=(MBF[0]&0x01)?1:0; MRB=(MBF[0]&0x02)?1:0;
          } break;
        }
    }
}

static int CLK(i32 x,i32 y,i32 w,i32 h){ return MLB&&!PMLB&&MX>=x&&MX<x+w&&MY>=y&&MY<y+h; }
static int HOV(i32 x,i32 y,i32 w,i32 h){ return MX>=x&&MX<x+w&&MY>=y&&MY<y+h; }

static u32 pci_rd(u8 bus,u8 dev,u8 fn,u8 off){ outl(0xCF8,0x80000000u|((u32)bus<<16)|((u32)dev<<11)|((u32)fn<<8)|(off&0xFC)); return inl(0xCFC); }
static int USB_OK=0; static char USB_NM[32]="USB Surucu (8GB)";
static void pci_scan(void){
    for(int b=0;b<4;b++) for(int d=0;d<32;d++){
        u32 id=pci_rd(b,d,0,0); if((id&0xFFFF)==0xFFFF) continue;
        u32 cls=pci_rd(b,d,0,8); u8 cc=(u8)(cls>>24),sc2=(u8)(cls>>16);
        if(cc==0x0C&&sc2==0x03) USB_OK=1;
    }
}

/* ================================================================
   ORTAK UI BİLEŞENLERİ
   ================================================================ */
static int BTN1(i32 x,i32 y,i32 w,i32 h,const char*lbl,u32 bg,u32 fg){
    u32 c=HOV(x,y,w,h)?(bg==CBL?C2:0xFFD0D0D0u):bg;
    rr(x,y,w,h,4,c); dsc(x,y+(h-8)/2,w,lbl,fg,c,1); return CLK(x,y,w,h);
}
static void WLOGO(i32 cx,i32 cy,i32 sz){
    i32 h=sz/2,g=2; fr(cx-h,cy-h,h-g,h-g,0xFFF35325u); fr(cx+g, cy-h,h-g,h-g,0xFF81BC06u); fr(cx-h,cy+g, h-g,h-g,0xFF05A6F0u); fr(cx+g, cy+g, h-g,h-g,0xFFFFBA08u);
}
static void CUR(void){
    static const u8 cur[16][12]={ {1,0},{1,1,0},{1,2,1,0},{1,2,2,1,0},{1,2,2,2,1,0},{1,2,2,2,2,1,0},{1,2,2,2,2,2,1,0},{1,2,2,2,2,2,2,1,0},{1,2,2,2,2,2,2,2,1,0},{1,2,2,2,2,1,1,1,1,1,0},{1,2,2,1,2,2,1,0},{1,2,1,0,1,2,2,1,0},{1,1,0,0,1,2,2,1,0},{0,0,0,0,0,1,2,2,1,0},{0,0,0,0,0,1,2,2,1,0},{0,0,0,0,0,0,1,1,0} };
    for(int r=0;r<16;r++) for(int c=0;c<12;c++){ i32 px=MX+c, py=MY+r; if((u32)px>=SW||(u32)py>=SH) continue; if(cur[r][c]==1) pp(px,py,CK); else if(cur[r][c]==2) pp(px,py,CW); }
}

/* ================================================================
   UYGULAMA / DOSYA SİSTEMİ
   ================================================================ */
typedef struct{char n[20];int inst;u32 col;} App;
static App AP[8]={
    {"Mesajlar",1,0xFF0078D4u},{"Terminal",1,0xFF1A1A1Au},
    {"Kamera",  1,0xFF107C10u},{"Harita",  1,0xFFD13438u},
    {"Hesap M.",1,0xFF8B008Bu},{"Oyunlar", 0,0xFFFF8C00u},
    {"Tarayici",0,0xFF005FB8u},{"Ayarlar", 1,0xFF606060u},
};

typedef struct{char n[32];int d;} FSE;
static FSE LFS[]={ {"Masaustu",1},{"Belgeler",1},{"Indirmeler",1},{"Resimler",1},{"Muzik",1},{"wind_os.cfg",0},{"README.txt",0} };
static FSE UFS[]={ {"ChromeSetup.wind",0},{"Notepad.exe",0},{"Araclar.deb",0} }; /* .exe, .wind, .deb dosyaları eklendi */

/* ================================================================
   PENCERE YÖNETİMİ DEĞİŞKENLERİ
   ================================================================ */
static int DR=0;
static int FO=0,FU=0; static i32 FX=100,FY=80; static int FD=0; static i32 FDX=0,FDY=0; static int FS=-1;
static int CALC_OPEN=0; static i32 CX=300, CY=150, CW_W=240, CW_H=340; static int CDrag=0; static i32 CDX=0, CDY=0; static char CALC_DISP[32]="0"; static int CALC_LEN=1;
static int UTCK=0;

static int BROWSER_OPEN = 0;
static i32 BX=150, BY=100, BW_W=700, BW_H=500;
static int BDrag=0; static i32 BDX=0, BDY=0;

/* ================================================================
   UYGULAMALAR
   ================================================================ */
static void SYSTEM_MONITOR(void) {
    i32 mx = SW - 210, my = 100, mw = 200, mh = 140;
    rr(mx, my, mw, mh, 8, 0xBB111122u); ds(mx + 15, my + 15, "SISTEM MONITORU", CW, 0xBB111122u, 1);
    fr(mx + 10, my + 30, mw - 20, 1, 0xFF444455u);
    SYS_TICK++; char uptime_str[16]; itoa(SYS_TICK / 250, uptime_str);
    ds(mx + 15, my + 45, "CPU Yuku  : %12", 0xFF4CAF50u, 0xBB111122u, 1);
    ds(mx + 15, my + 65, "Bellek    : 48 MB / 256 MB", 0xFFFF9800u, 0xBB111122u, 1);
    ds(mx + 15, my + 85, "Uptime    : ", CW, 0xBB111122u, 1); ds(mx + 100, my + 85, uptime_str, CW, 0xBB111122u, 1); ds(mx + 130, my + 85, "sn", CW, 0xBB111122u, 1);
    ds(mx + 15, my + 105, "VRAM(Res) : 1024x768x32", 0xFF2196F3u, 0xBB111122u, 1);
}

static void CALCULATOR(void) {
    if (!CALC_OPEN) return;
    if (!CDrag && MLB && !PMLB && MY >= CY && MY < CY + 30 && MX >= CX && MX < CX + CW_W) { CDrag = 1; CDX = MX - CX; CDY = MY - CY; }
    if (CDrag) { if (MLB) { CX = MX - CDX; CY = MY - CDY; if(CX<0)CX=0; if(CY<0)CY=0; if(CX>SW-CW_W)CX=SW-CW_W; if(CY>SH-CW_H)CY=SH-CW_H; } else CDrag = 0; }
    fr(CX + 4, CY + 4, CW_W, CW_H, 0x88000000u); rr(CX, CY, CW_W, CW_H, 8, 0xFFF3F3F3u); rb(CX, CY, CW_W, CW_H, CMG, 1);
    fr(CX, CY, CW_W, 30, 0xFFE0E0E0u); ds(CX + 10, CY + 10, "Hesap Makinesi", CK, 0xFFE0E0E0u, 1);
    if (BTN1(CX + CW_W - 30, CY + 5, 20, 20, "X", CRD, CW)) CALC_OPEN = 0;
    rr(CX + 15, CY + 45, CW_W - 30, 50, 4, CW); rb(CX + 15, CY + 45, CW_W - 30, 50, CMG, 1);
    ds(CX + CW_W - 25 - (CALC_LEN * 16), CY + 60, CALC_DISP, CK, CW, 2);
    const char* keys[16] = { "7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+" };
    for (int i = 0; i < 16; i++) {
        int kx = CX + 15 + ((i % 4) * 55), ky = CY + 110 + ((i / 4) * 50); u32 btnColor = ((i%4) == 3 || i == 12 || i == 14) ? 0xFFE0E0E0u : CW;
        if (BTN1(kx, ky, 45, 40, keys[i], btnColor, CK)) {
            if (keys[i][0] == 'C') { CALC_DISP[0] = '0'; CALC_DISP[1] = 0; CALC_LEN = 1; } 
            else if (CALC_LEN < 12 && keys[i][0] != '=') {
                if (CALC_LEN == 1 && CALC_DISP[0] == '0') CALC_DISP[0] = keys[i][0]; else { CALC_DISP[CALC_LEN] = keys[i][0]; CALC_DISP[CALC_LEN+1] = 0; CALC_LEN++; }
            }
        }
    }
}

static void BROWSER(void) {
    if (!BROWSER_OPEN) return;
    if (!BDrag && MLB && !PMLB && MY >= BY && MY < BY + 35 && MX >= BX && MX < BX + BW_W) { BDrag = 1; BDX = MX - BX; BDY = MY - BY; }
    if (BDrag) { if (MLB) { BX = MX - BDX; BY = MY - BDY; if(BX<0)BX=0; if(BY<0)BY=0; if(BX>SW-BW_W)BX=SW-BW_W; if(BY>SH-BW_H)BY=SH-BW_H; } else BDrag = 0; }

    fr(BX + 5, BY + 5, BW_W, BW_H, 0x88000000u); fr(BX, BY, BW_W, BW_H, CW); rb(BX, BY, BW_W, BW_H, CMG, 1);
    fr(BX, BY, BW_W, 35, 0xFFDFE1E5u); 
    rr(BX + 10, BY + 8, 160, 27, 4, CW); fr(BX + 10, BY + 30, 160, 5, CW); circ(BX + 22, BY + 20, 6, CRD); ds(BX + 35, BY + 16, "Yeni Sekme", CK, CW, 1);
    if (BTN1(BX + BW_W - 35, BY + 8, 25, 20, "X", CRD, CW)) BROWSER_OPEN = 0;

    fr(BX, BY + 35, BW_W, 40, CW); fr(BX, BY + 74, BW_W, 1, CLG); 
    ds(BX + 15, BY + 48, "<  >", CGY, CW, 1); rr(BX + 55, BY + 42, BW_W - 80, 26, 13, 0xFFF1F3F4u); ds(BX + 70, BY + 51, "Google'da arayin veya bir URL yazin", CGY, 0xFFF1F3F4u, 1);

    i32 pageY = BY + 75; i32 logoX = BX + (BW_W / 2) - 80;
    dsc(logoX, pageY + 120, 160, "Google", 0xFF4285F4u, CW, 4); 
    rr(BX + (BW_W / 2) - 200, pageY + 180, 400, 44, 22, CW); rb(BX + (BW_W / 2) - 200, pageY + 180, 400, 44, CLG, 2); ds(BX + (BW_W / 2) - 170, pageY + 196, "Wind OS icin web arama...", CMG, CW, 1);
    rr(BX + (BW_W / 2) - 140, pageY + 240, 130, 36, 4, 0xFFF8F9FAu); dsc(BX + (BW_W / 2) - 140, pageY + 252, 130, "Google'da Ara", CK, 0xFFF8F9FAu, 1);
    rr(BX + (BW_W / 2) + 10, pageY + 240, 130, 36, 4, 0xFFF8F9FAu); dsc(BX + (BW_W / 2) + 10, pageY + 252, 130, "Kendimi Sansli", CK, 0xFFF8F9FAu, 1);
}

/* ================================================================
   DOSYA YÖNETİCİSİ (.wind, .exe, .deb Kontrolü Eklendi)
   ================================================================ */
static void FILEMGR(void){
    if(!FO) return;
    i32 fw=630,fh=430,fx=FX,fy=FY;
    fr(fx+4,fy+4,fw,fh,0x88000000u); fr(fx,fy,fw,fh,CW); rb(fx,fy,fw,fh,CBR,1);

    fr(fx,fy,fw,36,CDG); circ(fx+14,fy+18,8,CRD); circ(fx+34,fy+18,8,COR); circ(fx+54,fy+18,8,CGN);
    if(CLK(fx+6,fy+10,16,16)){FO=0;return;} ds(fx+70,fy+13,"Dosya Yoneticisi",CW,CDG,1);

    fr(fx,fy+36,fw,28,CLG);
    if(FU){ ds(fx+8,fy+43,"Bu Bilgisayar > USB Surucu",CDG,CLG,1); if(CLK(fx+8,fy+38,160,20)) FU=0; } 
    else ds(fx+8,fy+43,"Bu Bilgisayar",CDG,CLG,1);
    
    i32 sb=132; fr(fx,fy+64,sb,fh-64,0xFFF0F0F4u);
    const char*si[]={"Bu Bilgisayar","Masaustu","Belgeler","Resimler","Muzik"};
    for(int i=0;i<5;i++) ds(fx+8,fy+74+i*20,si[i],CDG,0xFFF0F0F4u,1);
    
    if(USB_OK){ circ(fx+12,fy+180,7,CBL); ds(fx+24,fy+175,USB_NM,CBL,0xFFF0F0F4u,1); if(CLK(fx+6,fy+168,sb-6,22)) FU=1; }

    i32 cx2=fx+sb+8,cy2=fy+70; FSE *en=FU?UFS:LFS; int cnt=FU?3:7;
    
    for(int i=0;i<cnt;i++){
        i32 ex=cx2+(i%4)*120,ey=cy2+(i/4)*104;
        if(ex+102>fx+fw||ey+92>fy+fh) continue;
        u32 bg=(FS==i)?CLL:CW;
        fr(ex,ey,102,90,bg); rb(ex,ey,102,90,CBR,1);
        
        int len = klen(en[i].n);
        int isExe = (len > 4 && en[i].n[len-1]=='e' && en[i].n[len-2]=='x' && en[i].n[len-3]=='e');
        int isWind = (len > 5 && en[i].n[len-1]=='d' && en[i].n[len-2]=='n' && en[i].n[len-3]=='i' && en[i].n[len-4]=='w');
        int isDeb = (len > 4 && en[i].n[len-1]=='b' && en[i].n[len-2]=='e' && en[i].n[len-3]=='d');

        if(en[i].d){
            fr(ex+16,ey+8,60,18,0xFFFFD700u); fr(ex+8,ey+22,74,42,0xFFFFE44Du);
        } else {
            fr(ex+20,ey+8,50,50,CW); rb(ex+20,ey+8,50,50,CMG,1); fr(ex+52,ey+8,18,16,CLG);
            
            if(isExe){
                fr(ex+21,ey+38,48,14,CRD); dsc(ex+21,ey+40,48,".exe",CW,CRD,1);
            } else if (isWind) {
                circ(ex+45, ey+25, 12, CBL);
                fr(ex+21,ey+38,48,14,CBL); dsc(ex+21,ey+40,48,".wind",CW,CBL,1);
            } else if (isDeb) {
                circ(ex+45, ey+25, 12, 0xFF9C27B0u); /* Mor Renk */
                fr(ex+21,ey+38,48,14,0xFF9C27B0u); dsc(ex+21,ey+40,48,".deb",CW,0xFF9C27B0u,1);
            }
        }
        char sn[15]={0};
        if(len>13){mcpy(sn,en[i].n,11);sn[11]='.';sn[12]='.';} else kcpy(sn,en[i].n);
        dsc(ex,ey+74,102,sn,CDG,bg,1);
        
        if(CLK(ex,ey,102,90)){
            FS=i;
            if(!en[i].d && (isExe || isWind || isDeb)){
                AP[6].inst = 1; /* Tıklandığında Tarayıcı Yüklensin */
            }
        }
    }
    fr(fx,fy+fh-22,fw,22,CLG); ds(fx+8,fy+fh-15,USB_OK?"USB takili":"USB takili degil",CGY,CLG,1);

    if(!FD&&MLB&&!PMLB&&MY>=fy&&MY<fy+36&&MX>=fx&&MX<fx+fw){FD=1;FDX=MX-fx;FDY=MY-fy;}
    if(FD){ if(MLB){ FX=MX-FDX; FY=MY-FDY; if(FX<0)FX=0; if(FY<0)FY=0; if(FX>(i32)SW-fw)FX=(i32)SW-fw; if(FY>(i32)SH-fh)FY=(i32)SH-fh; } else FD=0; }
}

/* ================================================================
   MASAÜSTÜ — ÇEKMECE
   ================================================================ */
static void DRAWER(void){
    if(!DR) return;
    i32 dh=452,dx=50,dw=(i32)SW-100; i32 dy=(i32)SH-52-dh;
    rr(dx,dy,dw,dh,10,0xEE1A1A28u); rb(dx,dy,dw,dh,0xFF3A3A5Au,1);
    dsc(dx,dy+14,dw,"UYGULAMALAR",CW,0xFF1A1A28u,1);
    
    for(int i=0;i<8;i++){
        i32 col=i%4,row=i/4; i32 ix=dx+28+col*((dw-56)/4); i32 iy=dy+58+row*140;
        u32 bg=AP[i].inst?AP[i].col:CTH; rr(ix,iy,78,78,10,bg);
        char ab[3]={AP[i].n[0],AP[i].n[1],0};
        dsc(ix,iy+35,78,ab,CW,bg,1); dsc(ix-4,iy+84,86,AP[i].n,CW,0xFF1A1A28u,1);
        
        if(AP[i].inst && CLK(ix, iy, 78, 78)) {
            if (i == 4) CALC_OPEN = 1;
            if (i == 6) BROWSER_OPEN = 1;
        }
    }
    if(MLB&&!PMLB&&!(MX>=dx&&MX<dx+dw&&MY>=dy&&MY<dy+dh) &&!(MX>=8&&MX<46&&MY>=(i32)SH-46&&MY<(i32)SH-8)) DR=0;
}

/* ================================================================
   MASAÜSTÜ (ANA EKRAN BİRLEŞTİRİCİSİ)
   ================================================================ */
static void DESKTOP(void){
    for(u32 y=0;y<SH-52;y++){
        u32 r=0x0AU+(u32)(y*16U/(SH-52U)); u32 g2=0x0AU+(u32)(y*6U/(SH-52U)); u32 b2=0x1EU+(u32)(y*24U/(SH-52U));
        fr(0,(i32)y,(i32)SW,1,0xFF000000u|(r<<16)|(g2<<8)|b2);
    }
    dsc(0,(i32)SH/2-10,(i32)SW,"Wind OS",0xFF1C1C3Au,0xFF000000u,2);

    rr((i32)SW-198,10,188,76,7,0xAA18203Au);
    ds((i32)SW-194,18,"Hava Durumu",CW,0xFF18203Au,1); ds((i32)SW-194,32,"Istanbul:  22 C",CW,0xFF18203Au,1);
    
    SYSTEM_MONITOR();  
    DRAWER();          
    FILEMGR();
    CALCULATOR();      
    BROWSER(); 
    
    fr(0,(i32)SH-52,(i32)SW,52,CTB); WLOGO(28,(i32)SH-26,22); if(CLK(8,(i32)SH-46,40,40)) DR=!DR;

    typedef struct{const char*l;int*f;}TB;
    TB tb[]={ {"DOS",&FO}, {"CAL",&CALC_OPEN}, {"CHR",&BROWSER_OPEN}, {"APP",&DR} };
    for(int i=0;i<4;i++){
        i32 tx=64+i*54,ty=(i32)SH-46;
        if (i == 2 && !AP[6].inst) continue;

        int act=(tb[i].f&&*tb[i].f);
        rr(tx,ty,46,36,4,act?CTH:CTB);
        if(act) fr(tx+17,ty+34,12,4,CBL);
        dsc(tx,ty+14,46,tb[i].l,CW,act?CTH:CTB,1);
        if(CLK(tx,ty,46,36)&&tb[i].f) *tb[i].f=!*tb[i].f;
    }

    ds((i32)SW-94,(i32)SH-40,"26:03",CW,CTB,1);
    if(++UTCK>3000){pci_scan();UTCK=0;}
}

/* ================================================================
   KERNEL_MAIN
   ================================================================ */
void kernel_main(multiboot_info_t *mbi){
    u8 bpp  = mbi->framebuffer_bpp; if(bpp==0) bpp=32; u32 Bpp = (u32)bpp / 8;
    FB  = (volatile u32*)(unsigned long)mbi->framebuffer_addr; SW  = mbi->framebuffer_width; SH  = mbi->framebuffer_height; SP  = mbi->framebuffer_pitch / Bpp;
    if(!FB || SW==0){ FB=(volatile u32*)0xFD000000u; SW=1024; SH=768; SP=1024; }

    mouse_init(); pci_scan(); gST = STATE_DESKTOP;

    while(1){
        mouse_poll(); kbd_poll();
        if(gST!=pST){DIRTY=1;pST=gST;}
        
        switch(gST){ case STATE_DESKTOP: DESKTOP(); break; }
        
        CUR(); swap_buffers();
        volatile int x=50000;while(x--)__asm__("nop");
    }
}
