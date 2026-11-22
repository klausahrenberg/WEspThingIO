#ifndef W_THING_GPIO_EDITOR_H
#define W_THING_GPIO_EDITOR_H

#include <Arduino.h>

//The editor of the gpio page. It is a script of its own, that is sent straight
//from the flash: copied into a page it would need a block of the heap, that an
//esp8266 hardly has. It works on the json of the device - it reads the text
//area of the page, builds the form from it and writes every change back, so the
//page is saved like the json page is.
const static char WT_GPIO_EDITOR_JS[] PROGMEM = R"=====(
(function(){
var mount=document.getElementById('ge'),ta=document.getElementById('json');
if(!mount||!ta)return;
ta.parentNode.style.display='none';
var big=mount.getAttribute('data-chip')==='32';
var PINS=big?[0,1,2,3,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33,34,35,36,39]:[0,1,2,3,4,5,12,13,14,15,16];
var NOTE=big?{}:{0:'boot',1:'TX',2:'boot LED',3:'RX',15:'boot',16:'wake'};
var P=['gpio','pin','GPIO'],INV=['inverted','bool','Inverted'];
var ID=['id','text','Name'],TI=['title','text','Title'];
var T={
led:['LED','out',[P,INV,['linkstate','bool','Show WiFi status']],{gpio:2,inverted:false,linkstate:false}],
relay:['Relay','out',[P,INV],{gpio:5,inverted:false}],
pwm:['PWM','out',[P],{gpio:5}],
ws2812:['WS2812','out',[P,['leds','num','Number of LEDs'],['rainbow','bool','Rainbow while on']],{gpio:3,leds:16,rainbow:true}],
pl9823:['PL9823','out',[P,['leds','num','Number of LEDs'],['rainbow','bool','Rainbow while on']],{gpio:3,leds:16,rainbow:true}],
serialdimmer:['Serial dimmer','out',[['channel','sel','Channel',[1,2]],['id','text','Brightness name'],['title','text','Brightness title']],{channel:1,id:'brightness',title:'Brightness'}],
button:['Button','in',[P,INV,['longPress','bool','Long press opens access point']],{gpio:0,inverted:true,longPress:false}],
'switch':['Switch','in',[P,INV],{gpio:4,inverted:false}],
sht30:['SHT30','sens',[['sda','pin','SDA'],['scl','pin','SCL'],['id','text','Temperature name'],['title','text','Temperature title'],['humidityId','text','Humidity name'],['humidityTitle','text','Humidity title']],{sda:4,scl:5,id:'temperature',title:'Temperature',humidityId:'humidity',humidityTitle:'Humidity'}],
htu21:['HTU21','sens',[['sda','pin','SDA'],['scl','pin','SCL'],['id','text','Temperature name'],['title','text','Temperature title'],['humidityId','text','Humidity name'],['humidityTitle','text','Humidity title']],{sda:4,scl:5,id:'temperature',title:'Temperature',humidityId:'humidity',humidityTitle:'Humidity'}],
group:['Group','grp',[ID,TI],{id:'on',title:'Channel',items:[]}],
mode:['Mode','grp',[ID,TI,['modeId','text','Mode name'],['modeTitle','text','Mode title']],{id:'on',title:'Switch',modeId:'mode',modeTitle:'Mode',items:[]}]
};
var BAR=[['Outputs',['led','relay','pwm','ws2812','pl9823','serialdimmer']],['Inputs',['button','switch']],['Sensors',['sht30','htu21']],['Grouped',['group','mode']]];
var items=[],sels=[];
var css=document.createElement('style');
css.textContent='.ge{margin:.6rem 0}.ge button{padding:.35rem .7rem}.ge .bar{display:flex;flex-wrap:wrap;gap:.4rem;align-items:center;margin:.4rem 0}.ge .bar b{margin-right:.3rem;color:var(--df)}.ge .card{background:var(--lb);border-radius:.7rem;margin:.5rem 0;overflow:hidden}.ge .card.in{margin-left:1rem}.ge .head{display:flex;gap:.4rem;align-items:center;padding:.5rem .8rem;border-bottom:1px solid var(--db)}.ge .head b{flex:1}.ge .body{display:flex;flex-wrap:wrap;gap:.6rem 1rem;padding:.6rem .8rem}.ge .f{display:flex;flex-direction:column;font-size:.85rem;gap:.15rem}.ge .c{flex-direction:row;align-items:center;gap:.4rem}.ge input[type=text],.ge input[type=number],.ge select{padding:.3rem;border:1px solid var(--df);border-radius:.3rem;background:var(--db);color:var(--lf);min-width:6rem}.ge .bad{outline:2px solid #e05252}.ge .sub{padding:0 .8rem .6rem}.ge .msg{color:#e05252}.ge .x{background:#c0392b}';
document.head.appendChild(css);
function el(t,c,x){var e=document.createElement(t);if(c)e.className=c;if(x!=null)e.textContent=x;return e;}
function btn(t,f,c){var b=el('button',c||'',t);b.type='button';b.onclick=f;return b;}
function clone(o){return JSON.parse(JSON.stringify(o));}
function clean(it){
var o={type:it.type},k,v;
for(k in it){
if(k=='type'||k=='_key')continue;
v=it[k];
if(k=='items'){
if(it.type=='mode'){o.items={};v.forEach(function(c,i){o.items[c._key||('item'+(i+1))]=clean(c);});}
else o.items=v.map(clean);
}else if(v!==''&&v!==null&&v!==undefined)o[k]=v;
}
return o;
}
function sync(){
ta.value=JSON.stringify(items.map(clean));
ta.dispatchEvent(new Event('input'));
check();
}
function cls(k){return k=='sda'?'a':(k=='scl'?'c':'g');}
function check(){
sels.forEach(function(a){
var v=a.s.value,bad=false;
if(v!=='')sels.forEach(function(b){if(b!==a&&b.s.value===v&&(cls(a.k)!=cls(b.k)||cls(a.k)=='g'))bad=true;});
a.s.className=(bad?'bad':'');
a.s.title=(bad?'This pin is used twice':'');
});
}
function field(it,f){
var k=f[0],ty=f[1],w=el('label','f'),i,s=el('span','',f[2]);
if(ty=='bool'){
i=el('input');i.type='checkbox';i.checked=!!it[k];
i.onchange=function(){it[k]=i.checked;sync();};
w.className='f c';w.appendChild(i);w.appendChild(s);return w;
}
w.appendChild(s);
if(ty=='pin'||ty=='sel'){
i=el('select');
var o=ty=='pin'?PINS.slice():f[3].slice();
if(ty=='pin'){i.appendChild(new Option('-',''));if(it[k]!=null&&o.indexOf(it[k])<0)o.push(it[k]);}
o.forEach(function(p){i.appendChild(new Option(ty=='pin'&&NOTE[p]?p+' ('+NOTE[p]+')':p,p));});
i.value=(it[k]==null?'':it[k]);
i.onchange=function(){it[k]=(i.value===''?null:+i.value);sync();};
if(ty=='pin')sels.push({s:i,k:k});
}else if(ty=='num'){
i=el('input');i.type='number';i.min=1;i.max=255;i.value=it[k]||'';
i.oninput=function(){it[k]=+i.value;sync();};
}else{
i=el('input');i.type='text';i.value=it[k]==null?'':it[k];
i.oninput=function(){it[k]=i.value;sync();};
}
w.appendChild(i);return w;
}
function make(type,list){
var d=clone(T[type][3]);d.type=type;
if(type=='group'||type=='mode'){
var n=items.filter(function(x){return x.type=='group'||x.type=='mode';}).length;
if(n)d.id='on_'+(n+1);
}
return d;
}
function card(it,list,idx,inner){
var t=T[it.type]||[it.type,'out',[],{}],c=el('div','card'+(inner?' in':''));
var h=el('div','head');h.appendChild(el('b','',t[0]));
h.appendChild(btn('▲',function(){if(idx>0){list.splice(idx-1,0,list.splice(idx,1)[0]);render();}}));
h.appendChild(btn('▼',function(){if(idx<list.length-1){list.splice(idx+1,0,list.splice(idx,1)[0]);render();}}));
h.appendChild(btn('✕',function(){list.splice(idx,1);render();},'x'));
c.appendChild(h);
var b=el('div','body');
if(inner&&inner=='mode')b.appendChild(field(it,['_key','text','Option name']));
t[2].forEach(function(f){b.appendChild(field(it,f));});
c.appendChild(b);
if(t[1]=='grp'){
var s=el('div','sub');
it.items.forEach(function(ch,i){s.appendChild(card(ch,it.items,i,it.type));});
var sel=el('select');sel.appendChild(new Option('+ Add item...',''));
['led','relay','pwm','ws2812','pl9823','serialdimmer','button','switch'].forEach(function(k){sel.appendChild(new Option(T[k][0],k));});
sel.onchange=function(){if(sel.value){var n=make(sel.value);if(it.type=='mode')n._key='Option '+(it.items.length+1);it.items.push(n);render();}};
s.appendChild(sel);c.appendChild(s);
}
return c;
}
function render(){
mount.innerHTML='';sels=[];
var w=el('div','ge');
var fr=el('div','bar');fr.appendChild(btn('Load json file',fromFile));w.appendChild(fr);
BAR.forEach(function(g){
var r=el('div','bar');r.appendChild(el('b','',g[0]));
g[1].forEach(function(k){r.appendChild(btn('+ '+T[k][0],function(){items.push(make(k));render();}));});
w.appendChild(r);
});
items.forEach(function(it,i){w.appendChild(card(it,items,i,false));});
mount.appendChild(w);
sync();
}
function load(){
items=ta.value.trim()?JSON.parse(ta.value):[];
(function norm(l){l.forEach(function(it){
if(it.items&&!Array.isArray(it.items)){var a=[];for(var k in it.items){var c=it.items[k];c._key=k;a.push(c);}it.items=a;}
if(it.items)norm(it.items);
if((it.type=='group'||it.type=='mode')&&!it.items)it.items=[];
});})(items);
}
function fromFile(){
var f=document.createElement('input');f.type='file';f.accept='.json,application/json';
f.onchange=function(){
if(!f.files.length)return;
var r=new FileReader();
r.onload=function(){
var old=ta.value;
try{ta.value=r.result;load();if(!Array.isArray(items))throw new Error('a list is expected');render();}
catch(e){ta.value=old;try{load();render();}catch(x){}alert('This is no valid configuration: '+e.message);}
};
r.readAsText(f.files[0]);
};
f.click();
}
try{load();render();}
catch(e){
mount.appendChild(el('div','msg','The configuration could not be read: '+e.message+'. Use the json page to repair it.'));
mount.appendChild(btn('Load json file',fromFile));
}
})();
)=====";

#endif
