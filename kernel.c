/*
 * Wind OS  -  kernel.c  v2 (GitHub Actions Fix)
 * DÜZELTİLEN HATALAR:
 * 1. Klavye: status ÖNCE okunuyor (mouse/kbd ayrımı doğru)
 * 2. Mouse : polling status-önce-oku + daha sağlam sync
 * 3. Framebuffer pitch: bpp kullanılarak hesaplanıyor
 * 4. Yazı çarpıklığı: SCR_P doğru hesaplandı
 * 5. pST, DIRTY değişkenleri ve """ hatası düzeltildi
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
/* SP = pitch in PIXELS */

/* ── DURUM ───────────────────────────────────────── */
static OS_State gST = STATE_SETUP_1_NAME;
static OS_State pST = STATE_SETUP_1_NAME;
static int DIRTY = 1;

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
#define C2   0xFF005A9Eu  /* koyu mavi hover */

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
    if(!n)return 0;
    u32 x=n,y=1;
    while(x>y){x=(x+y)/2;y=n/x;}
    return x;
}

/* ── 8x8 BITMAP FONT ─────────────────────────────── */
static const u8 F8[128][8]={
 [' ']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
 ['!']={0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
 ['"']={0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},
 ['#']={0x36,0x7F,0x36,0x36,0x7F,0x36,0x36,0x00},
 ['$']={0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},
 ['%']={0x63,0x33,0x18,0x0C,0x66,0x63,0x00,0x00},
 ['&']={0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},
 ['\'']={0x06,0x0C,0x00,0x00,0x00,0x00,0x00,0x00},
 ['(']={0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},
 [')']={0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
 ['*']={0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00},
 ['+']={0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
 [',']={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x0C},
 ['-']={0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
 ['.']={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
 ['/']={0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
 ['0']={0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},
 ['1']={0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
 ['2']={0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},
 ['3']={0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
 ['4']={0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},
 ['5']={0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
 ['6']={0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},
 ['7']={0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
 ['8']={0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},
 ['9']={0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
 [':']={0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
 [';']={0x00,0x18,0x18,0x00,0x18,0x18,0x0C,0x00},
 ['<']={0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},
 ['=']={0x00,0x3F,0x00,0x00,0x3F,0x00,0x00,0x00},
 ['>']={0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},
 ['?']={0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
 ['@']={0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},
 ['A']={0x0C,0x1E,0x33,0x3F,0x33,0x33,0x33,0x00},
 ['B']={0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},
 ['C']={0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
 ['D']={0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},
 ['E']={0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
 ['F']={0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},
 ['G']={0x3C,0x66,0x03,0x73,0x63,0x66,0x7C,0x00},
 ['H']={0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},
 ['I']={0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
 ['J']={0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},
 ['K']={0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
 ['L']={0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},
 ['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
 ['N']={0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},
 ['O']={0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
 ['P']={0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},
 ['Q']={0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
 ['R']={0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},
 ['S']={0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
 ['T']={0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
 ['U']={0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
 ['V']={0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},
 ['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
 ['X']={0x63,0x63,0x36,0x1C,0x36,0x63,0x63,0x00},
 ['Y']={0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
 ['Z']={0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},
 ['[']={0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00},
 ['\\']={0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
 [']']={0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00},
 ['^']={0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00},
 ['_']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
 ['`']={0x06,0x0C,0x18,0x00,0x00,0x00,0x00,0x00},
 ['a']={0x00,0x1E,0x30,0x3E,0x33,0x33,0x6E,0x00},
 ['b']={0x07,0x06,0x3E,0x66,0x66,0x66,0x3B,0x00},
 ['c']={0x00,0x1E,0x33,0x03,0x03,0x33,0x1E,0x00},
 ['d']={0x38,0x30,0x3E,0x33,0x33,0x33,0x6E,0x00},
 ['e']={0x00,0x1E,0x33,0x3F,0x03,0x33,0x1E,0x00},
 ['f']={0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00},
 ['g']={0x00,0x6E,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['h']={0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00},
 ['i']={0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00},
 ['j']={0x18,0x00,0x18,0x18,0x18,0x1B,0x1B,0x0E},
 ['k']={0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00},
 ['l']={0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
 ['m']={0x00,0x33,0x7F,0x7F,0x6B,0x63,0x63,0x00},
 ['n']={0x00,0x1F,0x33,0x33,0x33,0x33,0x33,0x00},
 ['o']={0x00,0x1E,0x33,0x33,0x33,0x33,0x1E,0x00},
 ['p']={0x00,0x3B,0x66,0x66,0x3E,0x06,0x06,0x0F},
 ['q']={0x00,0x6E,0x33,0x33,0x3E,0x30,0x30,0x78},
 ['r']={0x00,0x3B,0x6E,0x66,0x06,0x06,0x0F,0x00},
 ['s']={0x00,0x3E,0x03,0x1E,0x30,0x33,0x1E,0x00},
 ['t']={0x08,0x3E,0x0C,0x0C,0x0C,0x2C,0x18,0x00},
 ['u']={0x00,0x33,0x33,0x33,0x33,0x33,0x6E,0x00},
 ['v']={0x00,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},
 ['w']={0x00,0x63,0x6B,0x7F,0x7F,0x36,0x36,0x00},
 ['x']={0x00,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},
 ['y']={0x00,0x33,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['z']={0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00,0x00},
 ['{']={0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00},
 ['|']={0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
 ['}']={0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00},
 ['~']={0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00},
};

/* ================================================================
   ÇİZİM (BOUNDS-CHECKED)
   ================================================================ */
static inline void pp(i32 x,i32 y,u32 c){
    if((u32)x<SW&&(u32)y<SH) FB[(u32)y*SP+(u32)x]=c;
}
static void fr(i32 x,i32 y,i32 w,i32 h,u32 c){
    if(w<=0||h<=0) return;
    i32 x1=x<0?0:x, y1=y<0?0:y;
    i32 x2=x+w>(i32)SW?(i32)SW:x+w;
    i32 y2=y+h>(i32)SH?(i32)SH:y+h;
    for(i32 j=y1;j<y2;j++)
        for(i32 i=x1;i<x2;i++)
            FB[(u32)j*SP+(u32)i]=c;
}
static void rb(i32 x,i32 y,i32 w,i32 h,u32 c,i32 t){
    fr(x,y,w,t,c); fr(x,y+h-t,w,t,c);
    fr(x,y,t,h,c); fr(x+w-t,y,t,h,c);
}
static void circ(i32 cx,i32 cy,i32 r,u32 c){
    if(r<=0) return;
    for(i32 dy=-r;dy<=r;dy++)
        for(i32 dx=-r;dx<=r;dx++)
            if(dx*dx+dy*dy<=r*r) pp(cx+dx,cy+dy,c);
}
/* Yuvarlatılmış dikdörtgen */
static void rr(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c){
    if(r>w/2) r=w/2; if(r>h/2) r=h/2;
    fr(x+r,y,w-2*r,h,c);
    fr(x,y+r,r,h-2*r,c); fr(x+w-r,y+r,r,h-2*r,c);
    circ(x+r,y+r,r,c);   circ(x+w-r-1,y+r,r,c);
    circ(x+r,y+h-r-1,r,c); circ(x+w-r-1,y+h-r-1,r,c);
}
/* WiFi yayı */
static void warc(i32 cx,i32 cy,i32 r,i32 t,u32 c){
    for(i32 dx=-r;dx<=r;dx++){
        i32 dy=-(i32)isqrt((u32)(r*r-dx*dx));
        for(i32 k=0;k<t;k++) pp(cx+dx,cy+dy-k,c);
    }
}
/* Karakter çiz (scale=piksel büyüklüğü) */
static void dc(i32 x,i32 y,char ch,u32 fg,u32 bg,i32 sc){
    if((u8)ch>=128) ch='?';
    const u8 *g=F8[(u8)ch];
    for(i32 row=0;row<8;row++)
        for(i32 col=0;col<8;col++)
            fr(x+col*sc,y+row*sc,sc,sc,
               (g[row]&(1<<(7-col)))?fg:bg);
}
/* String çiz */
static void ds(i32 x,i32 y,const char*s,u32 fg,u32 bg,i32 sc){
    i32 cx=x;
    while(*s){
        if(*s=='\n'){cx=x;y+=8*sc;}
        else{dc(cx,y,*s,fg,bg,sc);cx+=8*sc;}
        s++;
    }
}
/* Ortalanmış string */
static void dsc(i32 x,i32 y,i32 w,const char*s,u32 fg,u32 bg,i32 sc){
    i32 tw=(i32)klen(s)*8*sc;
    if(tw<w) ds(x+(w-tw)/2,y,s,fg,bg,sc);
    else     ds(x,y,s,fg,bg,sc);
}

/* ================================================================
   PS/2 KLAVYE  — DÜZELTİLDİ: status ÖNCE okunuyor
   ================================================================ */
static u8 K_SH=0, K_CP=0;
static const char SCMAP[128]={
  0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,
  '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
  0,'a','s','d','f','g','h','j','k','l',';','\'','`',
  0,'\\','z','x','c','v','b','n','m',',','.','/',
  0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,'-',0,0,0,'+',0,0,0,0,0,0,0,0,0
};
static u8 kbd_poll(void){
    u8 st=inb(0x64);
    if(!(st&0x01)) return 0;
    /* veri yok                    */
    if( (st&0x20)){ inb(0x60);
        return 0; } /* MOUSE verisi → çöp */
    u8 sc=inb(0x60);
    if(sc&0x80){                  /* tuş bırakıldı               */
        u8 r=sc&0x7F;
        if(r==0x2A||r==0x36) K_SH=0;
        return 0;
    }
    if(sc==0x2A||sc==0x36){K_SH=1;return 0;}
    if(sc==0x3A){K_CP=!K_CP;return 0;}
    if(sc>=128) return 0;
    char c=SCMAP[sc]; if(!c) return 0;
    if(c>='a'&&c<='z'){ if(K_SH^K_CP) c-=32; }
    else if(K_SH){
        switch(c){
          case '1':c='!';break;
          case '2':c='@';break; case '3':c='#';break;
          case '4':c='$';break; case '5':c='%';break; case '6':c='^';break;
          case '7':c='&';break; case '8':c='*';break;
          case '9':c='(';break;
          case '0':c=')';break; case '-':c='_';break; case '=':c='+';break;
          case '[':c='{';break;
          case ']':c='}';break; case ';':c=':';break;
          case '\'':c='"';break;case ',':c='<';break; case '.':c='>';break;
          case '/':c='?';break; case '`':c='~';break; case '\\':c='|';break;
        }
    }
    return (u8)c;
}

/* ================================================================
   PS/2 MOUSE  — DÜZELTİLDİ: status ÖNCE, daha sağlam sync
   ================================================================ */
static i32 MX=512,MY=384,MLB=0,MRB=0,PMLB=0;
static u8  MCY=0; static i8 MBF[3]={0};
static int MOUSE_READY=0;

static void m_cmd_wait(void){u32 t=100000;while(t--&&(inb(0x64)&0x02));}
static void m_dat_wait(void){u32 t=100000;while(t--&&!(inb(0x64)&0x01));}
static void m_write(u8 v){m_cmd_wait();outb(0x64,0xD4);m_cmd_wait();outb(0x60,v);}
static u8   m_read (void){m_dat_wait();return inb(0x60);}

static void mouse_init(void){
    /* PS/2 controller reset + aux enable */
    m_cmd_wait(); 
    outb(0x64,0xA8);          /* aux port aç         */
    m_cmd_wait(); outb(0x64,0x20);          /* config oku          */
    m_dat_wait();
    u8 cfg=inb(0x60);
    cfg|=0x02; cfg&=~0x20;                  /* aux IRQ on, aux clock on */
    m_cmd_wait(); outb(0x64,0x60);
    m_cmd_wait(); outb(0x60,cfg);
    /* Mouse reset */
    m_write(0xFF);
    u8 ack=m_read();
    /* 0xFA = ACK */
    u8 ok =m_read();
    /* 0xAA = self-test OK */
    m_read();
    /* mouse ID = 0x00 */

    if(ack==0xFA && ok==0xAA){
        m_write(0xF6);
        m_read();            /* default settings */
        m_write(0xF4); m_read();
        /* enable reporting */
        MOUSE_READY=1;
    }
}

static void mouse_poll(void){
    if(!MOUSE_READY) return;
    /* Sadece mouse verisini oku, klavyeyi dokuma */
    for(int iter=0;iter<16;iter++){
        u8 st=inb(0x64);
        if(!(st&0x01)) break;              /* veri yok → dur      */
        if(!(st&0x20)){                    /* klavye verisi        */
            inb(0x60);
            /* sadece drain et      */
            continue;
        }
        u8 dat=inb(0x60);
        switch(MCY){
          case 0:
            if(!(dat&0x08)){MCY=0;break;}  /* sync bit yok → sıfırla */
            MBF[0]=(i8)dat;
            MCY=1; break;
          case 1: MBF[1]=(i8)dat; MCY=2; break;
          case 2: MBF[2]=(i8)dat;
            MCY=0;{
            /* Delta X / Y  (sign extension) */
            i32 dx=(i32)MBF[1];
            i32 dy=(i32)MBF[2];
            if(MBF[0]&0x10) dx|=(i32)0xFFFFFF00;
            if(MBF[0]&0x20) dy|=(i32)0xFFFFFF00;
            /* Taşma bayrakları */
            if(MBF[0]&0x40) dx=0;
            if(MBF[0]&0x80) dy=0;
            MX+=dx; MY-=dy;
            if(MX<0)MX=0;
            if(MY<0)MY=0;
            if(MX>=(i32)SW) MX=(i32)SW-1;
            if(MY>=(i32)SH) MY=(i32)SH-1;
            PMLB=MLB;
            MLB=(MBF[0]&0x01)?1:0;
            MRB=(MBF[0]&0x02)?1:0;
          } break;
        }
    }
}

static int CLK(i32 x,i32 y,i32 w,i32 h){
    return MLB&&!PMLB&&MX>=x&&MX<x+w&&MY>=y&&MY<y+h;
}
static int HOV(i32 x,i32 y,i32 w,i32 h){
    return MX>=x&&MX<x+w&&MY>=y&&MY<y+h;
}

/* ================================================================
   PCI / USB ALGILAMA
   ================================================================ */
static u32 pci_rd(u8 bus,u8 dev,u8 fn,u8 off){
    outl(0xCF8,0x80000000u|((u32)bus<<16)|((u32)dev<<11)|((u32)fn<<8)|(off&0xFC));
    return inl(0xCFC);
}
static int USB_OK=0;
static char USB_NM[32]="USB Surucu (8GB)";
static void pci_scan(void){
    for(int b=0;b<4;b++) for(int d=0;d<32;d++){
        u32 id=pci_rd(b,d,0,0);
        if((id&0xFFFF)==0xFFFF) continue;
        u32 cls=pci_rd(b,d,0,8);
        u8 cc=(u8)(cls>>24),sc2=(u8)(cls>>16);
        if(cc==0x0C&&sc2==0x03) USB_OK=1;
    }
}

/* ================================================================
   ORTAK UI
   ================================================================ */
static int BTN(i32 x,i32 y,i32 w,i32 h,const char*lbl,u32 bg,u32 fg){
    u32 c=HOV(x,y,w,h)?(bg==CBL?C2:0xFFD0D0D0u):bg;
    rr(x,y,w,h,5,c);
    dsc(x,y+(h-16)/2,w,lbl,fg,c,2);
    return CLK(x,y,w,h);
}
static int BTN1(i32 x,i32 y,i32 w,i32 h,const char*lbl,u32 bg,u32 fg){
    u32 c=HOV(x,y,w,h)?(bg==CBL?C2:0xFFD0D0D0u):bg;
    rr(x,y,w,h,4,c);
    dsc(x,y+(h-8)/2,w,lbl,fg,c,1);
    return CLK(x,y,w,h);
}
static int TGL(i32 x,i32 y,int on){
    rr(x,y,46,24,12,on?CBL:CMG);
    circ(on?x+34:x+12,y+12,10,CW);
    return CLK(x,y,46,24);
}
static int CHK(i32 x,i32 y,int v){
    fr(x,y,20,20,CW); rb(x,y,20,20,v?CBL:CBR,1);
    if(v){
        for(i32 i=0;i<5;i++){pp(x+3+i,y+11+i,CBL);pp(x+4+i,y+11+i,CBL);}
        for(i32 i=0;i<7;i++){pp(x+8+i,y+15-i,CBL);pp(x+9+i,y+15-i,CBL);}
    }
    return CLK(x,y,20,20);
}
static void WLOGO(i32 cx,i32 cy,i32 sz){
    i32 h=sz/2,g=2;
    fr(cx-h,cy-h,h-g,h-g,0xFFF35325u);
    fr(cx+g, cy-h,h-g,h-g,0xFF81BC06u);
    fr(cx-h,cy+g, h-g,h-g,0xFF05A6F0u);
    fr(cx+g, cy+g, h-g,h-g,0xFFFFBA08u);
}
static void PROG(int cur){
    i32 by=(i32)SH-30;
    fr(0,by,(i32)SW,2,CLG);
    i32 step=(i32)SW/9;
    for(int i=0;i<7;i++){
        i32 px=step*(i+1),py=by+12;
        if(i<cur)        circ(px,py,6,CBL);
        else if(i==cur) {circ(px,py,8,CBL);circ(px,py,5,CW);}
        else            {circ(px,py,6,CLG);circ(px,py,5,CB);}
    }
}
static void FOOT(void){
    fr(0,(i32)SH-48,(i32)SW,48,CDG);
    WLOGO(30,(i32)SH-24,22);
    ds(46,(i32)SH-32,"Wind OS",CW,CDG,1);
}
/* Mouse imleci */
static void CUR(void){
    static const u8 cur[16][12]={
        {1,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,2,1,0,0,0,0,0,0,0},
        {1,2,2,2,2,1,0,0,0,0,0,0},
        {1,2,2,2,2,2,1,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,0,0,0,0},
        {1,2,2,2,2,2,2,2,1,0,0,0},
        {1,2,2,2,2,1,1,1,1,1,0,0},
        {1,2,2,1,2,2,1,0,0,0,0,0},
        {1,2,1,0,1,2,2,1,0,0,0,0},
        {1,1,0,0,1,2,2,1,0,0,0,0},
        {0,0,0,0,0,1,2,2,1,0,0,0},
        {0,0,0,0,0,1,2,2,1,0,0,0},
        {0,0,0,0,0,0,1,1,0,0,0,0},
    };
    for(int r=0;r<16;r++) for(int c=0;c<12;c++){
        i32 px=MX+c, py=MY+r;
        if((u32)px>=SW||(u32)py>=SH) continue;
        if(cur[r][c]==1) pp(px,py,CK);
        else if(cur[r][c]==2) pp(px,py,CW);
    }
}

/* ================================================================
   KURULUM DEĞİŞKENLERİ
   ================================================================ */
static char PCN[64]={0}; static int PCL=0;
static int RS=0, KS=0, NS=-1;
static char WPW[64]={0}; static int WPL=0, WIN=0;
static int PFB=1,PLO=1,PAD=1,PDG=0;
static int CE=1,CG=0,CS=0,CF=0;

/* ================================================================
   UYGULAMA / DOSYA SİSTEMİ
   ================================================================ */
typedef struct{char n[20];int inst;u32 col;} App;
static App AP[8]={
    {"Mesajlar",1,0xFF0078D4u},{"Terminal",1,0xFF1A1A1Au},
    {"Kamera",  1,0xFF107C10u},{"Harita",  1,0xFFD13438u},
    {"Hesap M.",1,0xFF8B008Bu},{"Oyunlar", 0,0xFFFF8C00u},
    {"Tarayici",1,0xFF005FB8u},{"Ayarlar", 1,0xFF606060u},
};

typedef struct{char n[32];int d;} FSE;
static FSE LFS[]={
    {"Masaustu",1},{"Belgeler",1},{"Indirmeler",1},
    {"Resimler",1},{"Muzik",   1},{"wind_os.cfg",0},{"README.txt",0},
};

static FSE UFS[]={
    {"Kurulum.exe",0},{"Oyun.exe",0},{"Notlar.txt",0},
};

/* ================================================================
   MASAÜSTÜ DURUM
   ================================================================ */
static int DR=0;
/* çekmece açık */
static int FO=0,FU=0;    /* dosya yöneticisi open/usb */
static i32 FX=80,FY=60;
static int FD=0; static i32 FDX=0,FDY=0;
static int FS=-1;
static int UTCK=0;

/* ================================================================
   EKRAN 1 — BİLGİSAYAR ADI
   ================================================================ */
static void S1(u8 key){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);

    /* Laptop simgesi */
    fr(85,145,210,150,CDG); fr(89,149,202,142,0xFF1A2A40u);
    circ(190,220,30,0xFF4FC3F7u);
    circ(179,211,4,CW); circ(201,211,4,CW);
    for(i32 i=-12;i<=12;i++) pp(190+i,230+(i*i/16),CW);
    fr(55,299,292,16,CDG);

    i32 rx=418,ry=65;
    ds(rx,ry,"Bilgisayariniza",CDG,CB,2);
    ry+=18;
    ds(rx,ry,"bir ad verin",CDG,CB,2); ry+=36;
    ds(rx,ry,"Bu cihazi agda nasil tanimlayacaksiniz?",CGY,CB,1); ry+=14;
    ds(rx,ry,"Ileride degistirebilirsiniz.",CGY,CB,1); ry+=32;

    /* Input kutu */
    fr(rx,ry,512,40,CW); rb(rx,ry,512,40,CBL,2);
    if(PCL) ds(rx+10,ry+14,PCN,CDG,CW,1);
    else    ds(rx+10,ry+14,"Bilgisayar-adi",CMG,CW,1);
    fr(rx+10+PCL*8,ry+10,2,20,CBL);

    ry+=54;
    ds(rx,ry,"* 19 karakterden az olmali.",CGY,CB,1);
    ry+=54;
    if(BTN(rx+300,ry,158,38,"Simdilik Atla",CLG,CDG)) gST=STATE_SETUP_2_REGION;
    if(BTN(rx+468,ry,80,38,"Ileri",CBL,CW))           gST=STATE_SETUP_2_REGION;
    
    FOOT(); PROG(0);

    if(key==8&&PCL>0){PCN[--PCL]=0;}
    else if(key>=32&&key<127&&PCL<18){PCN[PCL++]=(char)key;PCN[PCL]=0;}
}

/* ================================================================
   EKRAN 2 — BÖLGE
   ================================================================ */
static void S2(void){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);

    circ(195,280,90,0xFF2196F3u);
    fr(128,246,52,38,0xFF4CAF50u); fr(196,256,56,46,0xFF4CAF50u);
    fr(146,288,34,30,0xFF4CAF50u); fr(210,284,40,38,0xFF4CAF50u);
    for(i32 dy=-96;dy<=96;dy++)
        for(i32 dx=-96;dx<=96;dx++)
            if((u32)(dx*dx+dy*dy)>(u32)(90*90)) pp(195+dx,280+dy,CHD);

    i32 rx=418,ry=72;
    ds(rx,ry,"Bu dogru ulke/bolge mi?",CDG,CB,2); ry+=56;

    const char *re[]={"Turkiye","Turkce Q","Turkce F"};
    for(int i=0;i<3;i++){
        u32 bg=(RS==i)?CBL:CW, fg=(RS==i)?CW:CDG;
        fr(rx,ry,512,44,bg); rb(rx,ry,512,44,CBR,1);
        ds(rx+16,ry+14,re[i],fg,bg,1);
        if(CLK(rx,ry,512,44)) RS=i;
        ry+=50;
    }
    ry+=28;
    if(BTN(rx+432,ry,80,38,"Evet",CBL,CW)) gST=STATE_SETUP_3_KEYBOARD;
    FOOT(); PROG(1);
}

/* ================================================================
   EKRAN 3 — KLAVYE
   ================================================================ */
static void S3(void){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);

    const char*rw[]={"QWERTYUIOP","ASDFGHJKL","ZXCVBNM"};
    i32 ko[]={0,14,28};
    for(int r=0;r<3;r++) for(int k=0;rw[r][k];k++){
        i32 bx=42+ko[r]+k*30,by=218+r*37;
        fr(bx,by,26,28,CW); rb(bx,by,26,28,CMG,1);
        char ss[2]={(char)rw[r][k],0};
        ds(bx+9,by+10,ss,CDG,CW,1);
    }
    fr(78,330,236,28,CW); rb(78,330,236,28,CMG,1);
    ds(172,340,"SPACE",CMG,CW,1);

    i32 rx=418,ry=62;
    ds(rx,ry,"Bu, dogru klavye",CDG,CB,2); ry+=18;
    ds(rx,ry,"duzeni...",CDG,CB,2); ry+=36;
    ds(rx,ry,"Dogru ise devam edin,",CGY,CB,1); ry+=14;
    ds(rx,ry,"degilse asagidan secin.",CGY,CB,1); ry+=32;

    const char*la[]={"Q Layout","A_BD Layout","F Layout","S-E Layout","S-G Layout","S-H Layout"};
    for(int i=0;i<6;i++){
        u32 bg=(KS==i)?CLL:CW;
        fr(rx,ry,512,34,bg); rb(rx,ry,512,34,CBR,1);
        ds(rx+12,ry+12,la[i],CDG,bg,1);
        if(CLK(rx,ry,512,34)) KS=i;
        ry+=38;
    }
    ry+=14;
    if(BTN(rx+432,ry,80,38,"Evet",CBL,CW)) gST=STATE_SETUP_4_NETWORK;
    FOOT(); PROG(2);
}

/* ================================================================
   EKRAN 4 — AĞ
   ================================================================ */
static void S4(u8 key){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);

    circ(195,308,10,0xFF2196F3u);
    warc(195,318,32,6,0xFF2196F3u);
    warc(195,318,60,6,0xFF2196F3u);
    warc(195,318,88,6,0xFF2196F3u);

    i32 rx=418,ry=60;
    ds(rx,ry,"Hadi sizi bir aga",CDG,CB,2); ry+=18;
    ds(rx,ry,"baglayalim",CDG,CB,2); ry+=36;
    ds(rx,ry,"Baglanti olusturun veya mevcut aga baglanin.",CGY,CB,1);
    ry+=44;

    const char*ne[]={"Sky.Net-Giga","Sky.Net-Giga (Conc.)"};
    for(int i=0;i<2;i++){
        u32 bg=(NS==i)?CLL:CW;
        fr(rx,ry,512,52,bg); rb(rx,ry,512,52,CBR,1);
        circ(rx+22,ry+26,7,0xFF2196F3u);
        ds(rx+40,ry+20,ne[i],CDG,bg,1);
        if(CLK(rx,ry,512,52)){NS=i;WIN=1;}
        ry+=58;
    }
    if(WIN&&NS>=0){
        ry+=10;
        ds(rx,ry,"Parola:",CDG,CB,1); ry+=18;
        fr(rx,ry,512,38,CW); rb(rx,ry,512,38,CBL,2);
        char dots[65]={0}; for(int i=0;i<WPL&&i<48;i++) dots[i]='*';
        ds(rx+10,ry+14,dots,CDG,CW,1);
        fr(rx+10+WPL*8,ry+9,2,20,CBL);
        ry+=52;
        BTN1(rx+300,ry,100,36,"Iptal",CLG,CDG);
        if(BTN1(rx+410,ry,102,36,"Baglan",CBL,CW)){gST=STATE_SETUP_5_PRIVACY;WIN=0;}
        if(key==8&&WPL>0){WPW[--WPL]=0;}
        else if(key>=32&&key<127&&WPL<32){WPW[WPL++]=(char)key;WPW[WPL]=0;}
    } else {
        ry+=14;
        BTN1(rx+282,ry,118,36,"Aglar Goster",CLG,CDG);
        if(BTN1(rx+410,ry,102,36,"Ilerle",CBL,CW)) gST=STATE_SETUP_5_PRIVACY;
    }
    FOOT(); PROG(3);
}

/* ================================================================
   EKRAN 5 — GİZLİLİK
   ================================================================ */
static void S5(void){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);
    
    /* Kalkan */
    for(i32 dy=-80;dy<=80;dy++){
        i32 hw=dy<0?70:70-dy; if(hw<0)hw=0;
        fr(195-hw,282+dy,hw*2,1,0xFF1565C0u);
    }
    circ(195,282,20,0xFFFFD700u);
    fr(189,266,12,20,0xFF1565C0u);
    rb(188,264,14,22,0xFFFFD700u,3);

    i32 rx=418,ry=50;
    ds(rx,ry,"Cihaziniz icin",CDG,CB,2); ry+=18;
    ds(rx,ry,"gizlilik ayarlarini secin",CDG,CB,2); ry+=36;
    ds(rx,ry,"Asagidaki ozellikleri acip kapatabilirsiniz.",CGY,CB,1);
    ry+=30;

    typedef struct{const char*nm;const char*d;int*v;}PI;
    PI it[]={
        {"Geri bildirim",  "Sistem geri bildirimlerini yonet",        &PFB},
        {"Konum",          "Konumunuzu uygulamalar ile paylasin",     &PLO},
        {"Reklam Kimligi", "Kisisel reklamlar icin ID kullan",        &PAD},
        {"Teshis Sinyali", "Teshis verisini paylas",                  &PDG},
    };
    
    for(int i=0;i<4;i++){
        fr(rx,ry,512,58,CW); rb(rx,ry,512,58,CBR,1);
        ds(rx+12,ry+10,it[i].nm,CDG,CW,1); ds(rx+12,ry+24,it[i].d,CGY,CW,1);
        ds(rx+416,ry+24,*it[i].v?"Evet":"Hayir",*it[i].v?CBL:CGY,CW,1);
        if(TGL(rx+462,ry+17,*it[i].v)) *it[i].v=!*it[i].v;
        ry+=62;
    }
    ry+=10;
    ds(rx,ry,"Daha fazla bilgi edinin",CBL,CB,1); ry+=26;
    if(BTN(rx+412,ry,100,38,"Kabul Et",CBL,CW)) gST=STATE_SETUP_6_CUSTOMIZE;
    FOOT(); PROG(4);
}

/* ================================================================
   EKRAN 6 — ÖZELLEŞTIRME
   ================================================================ */
static void S6(void){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);
    
    u32 ic[]={0xFF2196F3u,0xFF4CAF50u,0xFFFF9800u,0xFF9C27B0u};
    i32 ip[][2]={{90,210},{252,196},{90,322},{252,308}};
    const char*il[]={"ENT","GAM","SCH","FAM"};
    for(int i=0;i<4;i++){circ(ip[i][0],ip[i][1],32,ic[i]);dsc(ip[i][0]-24,ip[i][1]-4,48,il[i],CW,ic[i],1);}

    i32 rx=418,ry=50;
    ds(rx,ry,"Deneyiminizi",CDG,CB,2); ry+=18;
    ds(rx,ry,"ozellestirelim",CDG,CB,2); ry+=36;
    ds(rx,ry,"Sectiginiz ilgi alanlarina gore oneriler",CGY,CB,1); ry+=14;
    ds(rx,ry,"ve ipuclari sunulabilir.",CGY,CB,1); ry+=28;

    typedef struct{const char*n;const char*d;int*v;u32 c;}CI;
    CI ci[]={
        {"Eglence","Video, muzik ve medyayi kesfet",&CE,0xFF2196F3u},
        {"Oyun",   "Oyunlari ve yeni surumleri takip et",&CG,0xFF4CAF50u},
        {"Okul",   "Odev ve projelerde verimli calis",&CS,0xFFFF9800u},
        {"Aile",   "Aile uyeleriyle guvenli kalin",&CF,0xFF9C27B0u},
    };
    
    for(int i=0;i<4;i++){
        fr(rx,ry,512,58,CW); rb(rx,ry,512,58,CBR,1);
        circ(rx+26,ry+29,18,ci[i].c);
        ds(rx+52,ry+12,ci[i].n,CDG,CW,1); ds(rx+52,ry+26,ci[i].d,CGY,CW,1);
        if(CHK(rx+484,ry+19,*ci[i].v)) *ci[i].v=!*ci[i].v;
        ry+=62;
    }
    ry+=12;
    BTN1(rx+290,ry,112,36,"Atla",CLG,CDG);
    if(BTN1(rx+412,ry,100,36,"Kabul Et",CBL,CW)) gST=STATE_SETUP_7_WELCOME;
    FOOT(); PROG(5);
}

/* ================================================================
   EKRAN 7 — HOŞ GELDİNİZ
   ================================================================ */
static void S7(void){
    fr(0,0,(i32)SW,(i32)SH,CB);
    fr(0,0,390,(i32)SH-48,CHD);
    FOOT();

    i32 cx=(i32)SW/2-210,cy=68;
    fr(cx+5,cy+5,420,210,0x44000000u);
    fr(cx,cy,420,210,CDG); rb(cx,cy,420,210,0xFF3A3A5Au,1);

    fr(cx+392,cy+6,22,22,CRD);
    dsc(cx+392,cy+10,22,"X",CW,CRD,1);
    if(CLK(cx+392,cy+6,22,22)) gST=STATE_DESKTOP;

    circ(cx+14,cy+18,8,CBL);
    ds(cx+28,cy+12,"HOS GELDINIZ",CW,CDG,1);
    ds(cx+12,cy+46,"Sisteme Hos Geldiniz!",CW,CDG,1);
    ds(cx+12,cy+62,"Wind OS hazir.",CMG,CDG,1);
    
    u32 ac[]={CBL,CGN,0xFF9C27B0u,CRD};
    const char*al[]={"MSG","TRM","HAR","KMR"};
    for(int i=0;i<4;i++){
        i32 ax=cx+12+i*100,ay=cy+112;
        rr(ax,ay,88,68,6,ac[i]);
        dsc(ax,ay+28,88,al[i],CW,ac[i],1);
    }

    dsc(0,336,(i32)SW,"Wind OS'e Hos Geldiniz!",CDG,CB,2);
    dsc(0,376,(i32)SW,PCL?PCN:"Kullanici",CGY,CB,1);

    if(BTN((i32)SW/2-120,430,240,48,"Masaustu Gir",CBL,CW)){
        pci_scan(); gST=STATE_DESKTOP;
    }
    PROG(6);
}

/* ================================================================
   MASAÜSTÜ — ÇEKMECE
   ================================================================ */
static void DRAWER(void){
    if(!DR) return;
    i32 dh=452,dx=50,dw=(i32)SW-100;
    i32 dy=(i32)SH-52-dh;
    rr(dx,dy,dw,dh,10,0xEE1A1A28u);
    rb(dx,dy,dw,dh,0xFF3A3A5Au,1);
    dsc(dx,dy+14,dw,"UYGULAMALAR",CW,0xFF1A1A28u,1);
    dsc(dx,dy+28,dw,"Tiklayin: yukle veya calistir",CMG,0xFF1A1A28u,1);
    
    for(int i=0;i<8;i++){
        i32 col=i%4,row=i/4;
        i32 ix=dx+28+col*((dw-56)/4);
        i32 iy=dy+58+row*140;
        u32 bg=AP[i].inst?AP[i].col:CTH;
        rr(ix,iy,78,78,10,bg);
        char ab[3]={AP[i].n[0],AP[i].n[1],0};
        dsc(ix,iy+35,78,ab,CW,bg,1);
        dsc(ix-4,iy+84,86,AP[i].n,CW,0xFF1A1A28u,1);
        if(!AP[i].inst){
            rr(ix+10,iy+98,58,22,4,CBL);
            dsc(ix+10,iy+104,58,"Yukle",CW,CBL,1);
            if(CLK(ix+10,iy+98,58,22)) AP[i].inst=1;
        }
    }
    /* Dışarı tık → kapat */
    if(MLB&&!PMLB&&!(MX>=dx&&MX<dx+dw&&MY>=dy&&MY<dy+dh)
       &&!(MX>=8&&MX<46&&MY>=(i32)SH-46&&MY<(i32)SH-8))
        DR=0;
}

/* ================================================================
   MASAÜSTÜ — DOSYA YÖNETİCİSİ
   ================================================================ */
static void FILEMGR(void){
    if(!FO) return;
    i32 fw=630,fh=430,fx=FX,fy=FY;
    fr(fx+4,fy+4,fw,fh,0x88000000u);
    fr(fx,fy,fw,fh,CW); rb(fx,fy,fw,fh,CBR,1);

    /* Başlık çubuğu */
    fr(fx,fy,fw,36,CDG);
    circ(fx+14,fy+18,8,CRD); circ(fx+34,fy+18,8,COR); circ(fx+54,fy+18,8,CGN);
    if(CLK(fx+6,fy+10,16,16)){FO=0;return;}
    ds(fx+70,fy+13,"Dosya Yoneticisi",CW,CDG,1);

    /* Adres çubuğu */
    fr(fx,fy+36,fw,28,CLG);
    if(FU){
        ds(fx+8,fy+43,"Bu Bilgisayar > USB Surucu",CDG,CLG,1);
        if(CLK(fx+8,fy+38,160,20)) FU=0;
    } else ds(fx+8,fy+43,"Bu Bilgisayar",CDG,CLG,1);
    
    /* Sol panel */
    i32 sb=132;
    fr(fx,fy+64,sb,fh-64,0xFFF0F0F4u);
    const char*si[]={"Bu Bilgisayar","Masaustu","Belgeler","Resimler","Muzik"};
    for(int i=0;i<5;i++) ds(fx+8,fy+74+i*20,si[i],CDG,0xFFF0F0F4u,1);
    
    if(USB_OK){
        circ(fx+12,fy+180,7,CBL);
        ds(fx+24,fy+175,USB_NM,CBL,0xFFF0F0F4u,1);
        if(CLK(fx+6,fy+168,sb-6,22)) FU=1;
    }

    /* İçerik */
    i32 cx2=fx+sb+8,cy2=fy+70;
    FSE *en=FU?UFS:LFS;
    int cnt=FU?3:7;
    
    for(int i=0;i<cnt;i++){
        i32 ex=cx2+(i%4)*120,ey=cy2+(i/4)*104;
        if(ex+102>fx+fw||ey+92>fy+fh) continue;
        u32 bg=(FS==i)?CLL:CW;
        fr(ex,ey,102,90,bg); rb(ex,ey,102,90,CBR,1);
        
        if(en[i].d){
            fr(ex+16,ey+8,60,18,0xFFFFD700u);
            fr(ex+8,ey+22,74,42,0xFFFFE44Du);
        } else {
            fr(ex+20,ey+8,50,50,CW); rb(ex+20,ey+8,50,50,CMG,1);
            fr(ex+52,ey+8,18,16,CLG);
            int nl=(int)klen(en[i].n);
            if(nl>4&&en[i].n[nl-1]=='e'&&en[i].n[nl-2]=='x'&&en[i].n[nl-3]=='e'){
                fr(ex+21,ey+38,48,14,CRD);
                dsc(ex+21,ey+40,48,".exe",CW,CRD,1);
            }
        }
        char sn[12]={0}; int nl2=(int)klen(en[i].n);
        if(nl2>10){mcpy(sn,en[i].n,9);sn[9]='.';sn[10]='.';}
        else kcpy(sn,en[i].n);
        dsc(ex,ey+74,102,sn,CDG,bg,1);
        
        if(CLK(ex,ey,102,90)){
            FS=i;
            int nl3=(int)klen(en[i].n);
            if(!en[i].d&&nl3>4&&en[i].n[nl3-1]=='e'&&en[i].n[nl3-2]=='x'&&en[i].n[nl3-3]=='e')
                for(int a=0;a<8;a++) if(!AP[a].inst){AP[a].inst=1;break;}
        }
    }
    /* Durum çubuğu */
    fr(fx,fy+fh-22,fw,22,CLG);
    ds(fx+8,fy+fh-15,USB_OK?"USB takili":"USB takili degil",CGY,CLG,1);

    /* Pencere sürükleme */
    if(!FD&&MLB&&!PMLB&&MY>=fy&&MY<fy+36&&MX>=fx&&MX<fx+fw){FD=1;FDX=MX-fx;FDY=MY-fy;}
    if(FD){
        if(MLB){
            FX=MX-FDX;
            FY=MY-FDY;
            if(FX<0)FX=0; if(FY<0)FY=0;
            if(FX>(i32)SW-fw)FX=(i32)SW-fw;
            if(FY>(i32)SH-fh)FY=(i32)SH-fh;
        } else FD=0;
    }
}

/* ================================================================
   MASAÜSTÜ
   ================================================================ */
static void DESKTOP(void){
    /* Gradyan arkaplan */
    for(u32 y=0;y<SH-52;y++){
        u32 r=0x0AU+(u32)(y*16U/(SH-52U));
        u32 g2=0x0AU+(u32)(y*6U/(SH-52U));
        u32 b2=0x1EU+(u32)(y*24U/(SH-52U));
        fr(0,(i32)y,(i32)SW,1,0xFF000000u|(r<<16)|(g2<<8)|b2);
    }
    dsc(0,(i32)SH/2-10,(i32)SW,"Wind OS",0xFF1C1C3Au,0xFF000000u,2);

    /* Hava widget */
    rr((i32)SW-198,10,188,76,7,0xAA18203Au);
    ds((i32)SW-194,18,"Hava Durumu ve Saat",CW,0xFF18203Au,1);
    ds((i32)SW-194,32,"Istanbul:  22 C",CW,0xFF18203Au,1);
    ds((i32)SW-194,46,"Parcali Bulutlu",CMG,0xFF18203Au,1);
    ds((i32)SW-194,62,"26:03  01.10.27",CW,0xFF18203Au,1);

    DRAWER();
    FILEMGR();
    
    /* ── GÖREV ÇUBUĞU ── */
    fr(0,(i32)SH-52,(i32)SW,52,CTB);
    WLOGO(28,(i32)SH-26,22);
    if(CLK(8,(i32)SH-46,40,40)) DR=!DR;

    typedef struct{const char*l;int*f;}TB;
    TB tb[]={{"FM",&FO},{"TB",NULL},{"MS",NULL},{"TM",NULL}};
    for(int i=0;i<4;i++){
        i32 tx=64+i*54,ty=(i32)SH-46;
        int act=(tb[i].f&&*tb[i].f);
        rr(tx,ty,46,36,4,act?CTH:CTB);
        if(act) fr(tx+17,ty+34,12,4,CBL);
        dsc(tx,ty+14,46,tb[i].l,CW,act?CTH:CTB,1);
        if(CLK(tx,ty,46,36)&&tb[i].f) *tb[i].f=!*tb[i].f;
    }

    ds((i32)SW-94,(i32)SH-40,"26:03",CW,CTB,1);
    ds((i32)SW-98,(i32)SH-24,"01.10.27",CW,CTB,1);

    if(USB_OK){
        circ((i32)SW-116,(i32)SH-26,6,CBL);
        ds((i32)SW-108,(i32)SH-30,"USB",CW,CTB,1);
    }

    if(++UTCK>3000){pci_scan();UTCK=0;}
}

/* ================================================================
   GECIKME
   ================================================================ */
static void delay(int n){volatile int x=n*5000;while(x--)__asm__("nop");}

/* ================================================================
   KERNEL_MAIN
   ================================================================ */
void kernel_main(multiboot_info_t *mbi){
    /*
     * PITCH DÜZELTMESİ:
     * framebuffer_pitch bayt cinsindendir.
     * framebuffer_bpp / 8 = bayt/piksel.
     * SP (pitch in pixels) = pitch_bytes / bytes_per_pixel
     */
    u8 bpp  = mbi->framebuffer_bpp;
    if(bpp==0) bpp=32;
    u32 Bpp = (u32)bpp / 8;            /* bytes per pixel */

    FB  = (volatile u32*)(unsigned long)mbi->framebuffer_addr;
    SW  = mbi->framebuffer_width;
    SH  = mbi->framebuffer_height;
    SP  = mbi->framebuffer_pitch / Bpp;
    /* DOĞRU pitch hesabı */

    /* GRUB VBE atamadıysa yedek */
    if(!FB || SW==0){
        FB=(volatile u32*)0xFD000000u;
        SW=1024; SH=768; SP=1024;
    }

    mouse_init();
    pci_scan();

    while(1){
        mouse_poll();
        u8 key=kbd_poll();
        
        if(gST!=pST){DIRTY=1;pST=gST;}

        switch(gST){
            case STATE_SETUP_1_NAME:      S1(key);  break;
            case STATE_SETUP_2_REGION:    S2();     break;
            case STATE_SETUP_3_KEYBOARD:  S3();     break;
            case STATE_SETUP_4_NETWORK:   S4(key);  break;
            case STATE_SETUP_5_PRIVACY:   S5();     break;
            case STATE_SETUP_6_CUSTOMIZE: S6();     break;
            case STATE_SETUP_7_WELCOME:   S7();     break;
            case STATE_DESKTOP:           DESKTOP();break;
        }
        CUR();
        delay(4);
    }
}
