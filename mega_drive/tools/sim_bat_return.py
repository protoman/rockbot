# Offline model of BAT BALL (DARK MINE): chase a player point, then fly back to
# the spawn with enemy.c step_axis; prints chases whose return ends elsewhere.
# usage (from mega_drive/): python3 tools/sim_bat_return.py 7 10   (ENEMY_SPEED_NUM DEN)
import re,sys
NUM,DEN=int(sys.argv[1]),int(sys.argv[2])
t=open('src/stage6_data.c').read()
m=re.search(r'terrain\[(\d+)\]\[(\d+)\] = \{(.*?)\};',t,re.S)
n,sz=int(m.group(1)),int(m.group(2))
v=[int(x) for x in re.findall(r'\b\d+\b',m.group(3))]
W=200
def terr(mp,tx,ty):
    if ty<0 or ty>=14: return 0
    if tx<0 or tx>=W: return 1
    return v[mp*sz+ty*W+tx]
def hits(mp,l,t,r,b):
    for ty in range(t>>4,((b-1)>>4)+1):
        for tx in range(l>>4,((r-1)>>4)+1):
            if terr(mp,tx,ty)==1: return True
    return False
BW,BH=27,21
def sgn(a): return (a>0)-(a<0)
class E: pass
def step(e,axis,dest):
    st=max(1,(1*NUM)//DEN)
    e.acc+=st*5
    s=e.acc//6; e.acc%=6
    pos=getattr(e,axis)
    if pos==dest: return True
    if s==0: return False
    d=sgn(dest-pos); nn=pos+d*s
    if (d>0 and nn>dest) or (d<0 and nn<dest): nn=dest
    if axis=='x':
        lead= nn+BW-1 if d>0 else nn
        if hits(e.mp,lead,e.y,lead,e.y+BH): return True
    else:
        edge=nn+BH if d>0 else nn
        if hits(e.mp,e.x,edge,e.x+BW,edge+1): return True
    setattr(e,axis,nn)
    return getattr(e,axis)==dest
def fly(e,dx,dy,maxf=3000):
    e.acc=0
    for f in range(maxf):
        a=step(e,'x',dx); b=step(e,'y',dy)
        if a and b: return f
    return -1
e=open('src/stage6_enemies.c').read()
sp=re.search(r'stage6_enemy_spawns\[[^\]]*\] = \{(.*?)\};',e,re.S).group(1)
bad=0;tot=0
for row in re.findall(r'\{ (\d+), (\d+), (\d+), (\d+),',sp):
    d,mp,x,y=map(int,row)
    if d!=3: continue
    for px in range(x-100,x+101,20):
        for py in range(0,200,16):
            b=E(); b.mp=mp;b.x=x;b.y=y
            fly(b,px,py)
            c=(b.x,b.y)
            fly(b,x,y)
            tot+=1
            if (b.x,b.y)!=(x,y):
                bad+=1
                if bad<=12: print('spawn',mp,x,y,'player',px,py,'chase end',c,'return end',(b.x,b.y))
print('bad',bad,'of',tot)
