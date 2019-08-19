
//todo how to prevent/handle backbutton on mobile platforms including browsers and webview? window.onbeforeunload=function(){return 0;}; OR history.pushState with '' empty URL (so backbutton does nothing) and window.onpopstate=function(){alert('quit?')}

//rarely use .disabled bc the coloring would be misleading
//rarely use <button>  bc the coloring would be misleading
//rarely use alert/prompt/confirm bc it prevents switching to other tabs in Chrome
//might use contextmenu event for some options, for mobile version you might need setTimeout on mousedown and clearTimeout on mouseup & mouseleave (for taphold event)
//try to make each page as short as possible (least height). so the scroll bar usually doesn't appear. don't be scared of big number of stacked pages
//use <a> can be better than <div>, main reason is that when you right click on <a> you have the option to open in new tab, which can be exploited for some feature using new tab
//using placeholder of input is okay, though it seems not recommended (mozilla.org)

//note sometimes you cannot use keyup bc if you go to alert page, and press Enter key to escape (keydown), then keyup fires immediately (so you go to alert page again if focus is on some text input)


window.onerror = function(messageOrEvent, source, lineno, colno, error) {
	alert(messageOrEvent+'\nSrc: '+source+'\nLine and Col: '+lineno+','+colno+'\n'+error.stack);
};

function assert(ex,err){
	if(!ex)throw new Error(err);
}


const defwm=new WeakMap;

const textinlenlimit=0x40;//debug change to bigger value (less than half of CHBUFSI)
const deftextenc=new TextEncoder;
const deftextdec=new TextDecoder;
function decodeutf8strs(uints){
	var end,off=0;
	var res=[];
	for(;;){
		end=uints.indexOf(0,off);
		if(end===-1){
			res.push(deftextdec.decode(uints.subarray(off)));
			return res;
		}else{
			res.push(deftextdec.decode(uints.subarray(off,end)));
		}
		off=end+1;
	}
}
function readlistofdv(odv,off,resarr){
	var si=odv.getUint32(off,true);
	off+=4;
	for(;si!==0;){
		si--;
		var len=odv.getUint32(off,true);
		off+=4;
		resarr.push(new DataView(odv.buffer, odv.byteOffset+off, len));
		off+=len;
	}
	return off;
}
function readlistofdvtillend(odv,off,resarr){
	for(;off!==odv.byteLength;){
		var len=odv.getUint32(off,true);
		off+=4;
		resarr.push(new DataView(odv.buffer, odv.byteOffset+off, len));
		off+=len;
	}
}

function isrealid(u8b){//if you are paranoid about byte representation at server app, you may send the NotRealID int64_t from server to webpage at startup, for memcmp
	return u8b[0] || u8b[1] || u8b[2] || u8b[3] || u8b[4] || u8b[5] || u8b[6] || u8b[7];
}
function readlistofidstr(u8b,resarr){
	for(var off=0;;){
		var id=u8b.subarray(off,off+8);
		off+=8;
		if(!isrealid(id)) return off;
		var end=u8b.indexOf(0,off);
		resarr.push({i:id,s:deftextdec.decode(u8b.subarray(off,end))});
		off=end+1;
	}
}
function readlistofidstrtillend(u8b,resarr){
	for(var off=0;off!==u8b.length;){
		var id=u8b.subarray(off,off+8);
		off+=8;
		var end=u8b.indexOf(0,off);
		resarr.push({i:id,s:deftextdec.decode(u8b.subarray(off,end))});
		off=end+1;
	}
}


const SYNOPSISLEN=0xff;
const MODULEUUIDSTR_ECSBAS_APP=new Uint8Array([0xfd, 0xcf, 0x17, 0xa4, 0xf4, 0xf9, 0x40, 0x28, 0xb7, 0x1b, 0xed, 0x64, 0x1b, 0x2c, 0x3c, 0xb5]);
const MODULEUUIDSTR_ECSBAS_GUI=new Uint8Array([0x7d, 0xfe, 0xf3, 0x20, 0x6b, 0x31, 0x42, 0x4f, 0x8b, 0x31, 0x58, 0x1e, 0x22, 0xd1, 0x21, 0x6f]);
const OFF=16;
const tc={
	specialized_sql:130,
	wordsense:69,
	text_note:68,
	bookmark:67,
	unusualcommands:66,
	listmodules:65,
	shutdown:64
};
const foldernmreg=/^[0-9a-z]([-_0-9a-z]*[0-9a-z])?$/;
const filenmreg=/^[0-9A-Za-z]([-_.0-9A-Za-z]*[0-9A-Za-z])?$/;
function alertfoldernmnotallowed(){alertmsg('Folder name must starts and ends with lower case or digit character. Other characters except - _ are not allowed.');}
function alertfilenmnotallowed(){alertmsg('Filename must starts and ends with alphanumeric character. Other characters except - _ . are not allowed.');}
function alertmsg(msg){
	setTimeout(()=>{//why use setTimeout: on firefox and ie, during keydown handler is executing, if focus() is called on another element, click event will fire
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem(''));
		elem.appendChild(document.createElement('p')).className='alerttip';
		elem.lastChild.textContent=msg;
		document.body.firstElementChild.style.display='none';
		insBeforeFirstChild(document.body,elem);
		elem.firstChild.focus();
	},0);
}
function confirmmsg(msg,cb){
	setTimeout(()=>{//why use setTimeout: on firefox and ie, during keydown handler is executing, if focus() is called on another element, click event will fire
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem(''));
		elem.appendChild(document.createElement('div')).className='fixedtip';
		elem.lastChild.textContent=msg;
		elem.appendChild(createAnchorClelemWithText('OK')).onclick=cb;
		bodyhidefirstinsertfirstfocusonfirst(elem);
	},0);
}
function promptmsg(msg,cb){
	setTimeout(()=>{//why use setTimeout: on firefox and ie, during keydown handler is executing, if focus() is called on another element, click event will fire
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem(''));
		elem.appendChild(document.createElement('label')).textContent=msg;
		document.body.firstElementChild.style.display='none';
		insBeforeFirstChild(document.body,elem);
		elem.lastChild.appendChild(createTextInputPreventDefaultWhenEnterKeyDown()).focus();
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			cb(this.value);
		};
	},0);
}
function showtrmsg(msg){//todo change to fixed tip displaying at bottom of page, removed with setTimeout
	alert(msg);
}

function bodyhidefirstinsertfirstfocusonfirst(elem){
	document.body.firstElementChild.style.display='none';
	insBeforeFirstChild(document.body,elem);
	focusonfirst();
}

function createAnchorClelemWithText(t){
	var tbr = createAnchorClelem();
	tbr.text=t;
	return tbr;
}
//var splitsecondgv;
function createAnchorClelem(){
	var tbr = createAnchorWithPhHref();
	//in order to force Chrome (especially mobile) to render before showing alert/confirm/prompt, stopImmediatePropagation() and then setTimeout() with click().
	//maybe you can write a function deferClick(anchorElemToBeDefered), and use the function for only anchors that will invoke alert/confirm/prompt.
	//? do you really want to use this?
	//tbr.addEventListener('click',function(e){
	//	if(splitsecondgv){splitsecondgv=undefined;}
	//	else{
	//		splitsecondgv=true;e.stopImmediatePropagation();var te=this;
	//		setTimeout(function(){te.click();},100);
	//	}
	//});//undone add an invisible ui freeze between the small gap
	tbr.className = 'clelem';
	return tbr;
}
const focuswm=new WeakMap;
function fnforesc(){
	document.body.removeChild(document.body.firstElementChild);document.body.firstElementChild.style.display='';
	var prevactiveelem=focuswm.get(document.body.firstElementChild);
	if(prevactiveelem && prevactiveelem!==document.body && document.body.contains(prevactiveelem)){
		prevactiveelem.focus();
	}else{
		focusonfirst();
	}
}
function createAnchorEscelem(loc){
	var prevactiveelem=document.activeElement;
	var tbr = createAnchorWithPhHref();
	tbr.className = 'escelem';
	tbr.text='\u00a0\u25c0 '+(loc===undefined?(prevactiveelem?prevactiveelem.textContent:''):loc);
	tbr.addEventListener('click',fnforesc);
	focuswm.set(document.body.firstElementChild,prevactiveelem);
	return tbr;
}
function makeClelem(elem){
	elem.className='clelem';
	elem.tabIndex=0;
	elem.addEventListener('keydown',function(ke){
		if(ke.keyCode===0x0d)this.click();
	});
	return elem;
}
function createAnchorIbclelemWithText(t){
	var tbr = createAnchorWithPhHref();
	tbr.className='ibclelem';
	tbr.text=t;
	return tbr;
}
function createDiv(t,cn){
	var tbr = document.createElement('div');
	tbr.textContent=t;
	tbr.className=cn;
	return tbr;
}

window.onkeydown=function(ke){
	switch(ke.keyCode){
	case 0x1b://esc
		var escelem=document.body.getElementsByClassName('escelem')[0];
		if(escelem) escelem.click();
		break;
	}
};

var timeoutidforfree;
function freezeui(){
	if(timeoutidforfree)
		clearTimeout(timeoutidforfree);//assuming that click can can fire between unfreeze-and-immediately-freeze-again is possible, and the javascript handler is fired after the freeze-again javascript is finished, then this is necessary
	else{
		//might need to add keyup mousedown mouseup in the future
		window.addEventListener('click',arg1PreventDefaultStopPropagation,true);
		window.addEventListener('keydown',arg1PreventDefaultStopPropagation,true);
		//? also disable scroll?
		document.body.insertAdjacentHTML('beforeend','<div style="position:fixed;top:0;left:0;width:100%;height:100%;opacity:0.5;background-color:#000"></div>');
	}
}

function unfreezeui(){
	timeoutidforfree=undefined;
	window.removeEventListener('click',arg1PreventDefaultStopPropagation,true);
	window.removeEventListener('keydown',arg1PreventDefaultStopPropagation,true);
	removeFromParentNode(document.body.lastChild);
}

function postreqforarraybuffer(bloborarr,cb,secondarg){
	var xhr = new XMLHttpRequest;
	xhr.open('POST','/');
	xhr.responseType='arraybuffer';
	xhr.setRequestHeader('Content-type','text/plain');//? no need to set charset=x-user-defined
	xhr.setRequestHeader('Accept','*/*');
	xhr.setRequestHeader('Accept-Language','*');
	xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
	//xhr.setRequestHeader('User-Agent','x');//?for now, it only works in firefox?
	freezeui();
	xhr.onreadystatechange = function () {
		if(xhr.readyState===XMLHttpRequest.DONE){
			switch(xhr.status){
			case 200:
				assert("application/octet-stream"===xhr.getResponseHeader('content-type'));//may be removed, bc you'll check uuid anyway
				var xres=xhr.response;
				assert(!strictCmpList(new Uint8Array(xres,0,OFF),MODULEUUIDSTR_ECSBAS_GUI,OFF));//may be not so necessary bc it's probably not a big problem for receiving unexpect data from unknown peer?
				timeoutidforfree=setTimeout(unfreezeui,0);//normally this is okay due to single thread nature of javascript, unless you have something like setTimeout
				cb(xres,secondarg);
				break;
			default:
				//todo if 404 then freeze everything, no more javascript running?
				throw new Error(xhr.status);
				break;
			}
		}
	};
	xhr.send(new Blob([MODULEUUIDSTR_ECSBAS_APP,bloborarr]));
}
function postreqforarraybufferwofreezeunfreeze(bloborarr,cb,secondarg){
	var xhr = new XMLHttpRequest;
	xhr.open('POST','/');
	xhr.responseType='arraybuffer';
	xhr.setRequestHeader('Content-type','text/plain');//? no need to set charset=x-user-defined
	xhr.setRequestHeader('Accept','*/*');
	xhr.setRequestHeader('Accept-Language','*');
	xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
	//xhr.setRequestHeader('User-Agent','x');//?for now, it only works in firefox?
	xhr.onreadystatechange = function () {
		if(xhr.readyState===XMLHttpRequest.DONE){
			switch(xhr.status){
			case 200:
				assert("application/octet-stream"===xhr.getResponseHeader('content-type'));//may be removed, bc you'll check uuid anyway
				var xres=xhr.response;
				assert(!strictCmpList(new Uint8Array(xres,0,OFF),MODULEUUIDSTR_ECSBAS_GUI,OFF));//may be not so necessary bc it's probably not a big problem for receiving unexpect data from unknown peer?
				cb(xres,secondarg);
				break;
			default:
				//todo if 404 then freeze everything, no more javascript running?
				throw new Error(xhr.status);
				break;
			}
		}
	};
	xhr.send(new Blob([MODULEUUIDSTR_ECSBAS_APP,bloborarr]));
}

const pred={};

function focusonfirst(){
	var firstfocus=document.body.querySelector('.clelem,select,input');//body.getElementsByClassName('clelem')[0];
	if(firstfocus) firstfocus.focus();
}

function makepredverbrow(useverbin,id,text){
	useverbin.style.display='none';
	useverbin.parentNode.insertBefore(createAnchorClelem(),useverbin);
	defwm.set(useverbin.previousSibling,id);
	useverbin.previousSibling.onclick=oc_chverb.bind(useverbin.previousSibling,useverbin);
	useverbin.previousSibling.text=text;
}
function addrowsforverb(sep){
	sep.parentNode.insertBefore(createAnchorClelem(),sep).text=sep.text;
	sep.previousSibling.onclick=oc_separator;
	sep.parentNode.insertBefore(createTextInputPreventDefaultWhenEnterKeyDown(),sep).placeholder='Use Verb: ';
	sep.previousSibling.focus();
	sep.previousSibling.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		switch(this.value){
		case ' '://top level verbs
			postreqforarraybuffer(new Uint8Array([tc.specialized_sql,4]),listverbsforsel,this);
			break;
		case 'w'://Is A Word Group
			makepredverbrow(this,pred.IS_A_NATURAL_LANG_WORD_SENSE,'Is A Word Group');
			setTimeout(this.previousSibling.focus.bind(this.previousSibling),0);
			break;
		case ''://Text
		case 't':
			makepredverbrow(this,pred.VERB_CONTAINS_TEXT,'Text');
			this.parentNode.insertBefore(createTextInputPreventDefaultWhenEnterKeyDown(),this).onkeydown=function(ke){
				if(ke.keyCode!==0x0d)return;
				search.h_submit.click();
			};
			this.previousSibling.focus();
			break;
		case '*':
			makepredverbrow(this,pred.ANY_USER_DEFINED_VERB,'*');
			setTimeout(this.previousSibling.focus.bind(this.previousSibling),0);
			break;
		case 'n':
			makepredverbrow(this,pred.IS_A_TEXT_NOTE,'Is A Text Note');
			setTimeout(this.previousSibling.focus.bind(this.previousSibling),0);
			break;
		case 'b':
			makepredverbrow(this,pred.IS_A_BOOKMARK,'Is A Bookmark');
			setTimeout(this.previousSibling.focus.bind(this.previousSibling),0);
			break;
		default:
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,10]),this.value]),listverbsforsel,this);
			break;
		}
	};
}
function seseparatorreadytosubmit(v){
	var tbr=v.replace(/\s+/g,'').toLowerCase();
	if(/^\)*(or|and|)\(*$/.test(tbr))return tbr;
}
function oc_separator(){
		var cothis=this;
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		elem.insertAdjacentHTML('beforeend', '<div class="fixedtip" >Format of separator is <b>\\)*(OR|AND|)\\(*</b> (case-insensitive). Empty separator is the same as <b>AND</b>.</div>');
		var setextin=elem.appendChild(createTextInputPreventDefaultWhenEnterKeyDown());
		setextin.value=this.text;
		elem.firstChild.onclick=function(){
			if(seseparatorreadytosubmit(setextin.value)===undefined)alertmsg('Invalid separator. (May temporarily leave it at that. But cannot submit it.)');
			cothis.text=setextin.value;
		};
		elem.appendChild(createAnchorClelem()).text='Add a row for verb';
		elem.lastChild.onclick=function(){
			fnforesc();
			addrowsforverb(cothis);
		};
		var useverbin=this.previousSibling;
		if(!useverbin.classList.contains('escelem')){
			elem.appendChild(createAnchorClelem()).text='Remove the row above (and the separator)';
			elem.lastChild.onclick=seremoverownsepbelow.bind(undefined,useverbin);
		}
		var useverbin=this.nextSibling;
		if(useverbin.tagName!=='DIV'){
			if(useverbin.tagName!=='INPUT'){
				useverbin=useverbin.nextSibling;
				if(useverbin.nextSibling.tagName==='INPUT') useverbin=useverbin.nextSibling;
			}
			elem.appendChild(createAnchorClelem()).text='Remove the row below (and the separator)';
			elem.lastChild.onclick=seremoverownsepabove.bind(undefined,useverbin);
		}
		bodyhidefirstinsertfirstfocusonfirst(elem);
}

var textnote={};
var search={};

function moduleview(nm){
	switch(nm){
	case 'search':
		var elem=document.createElement('div');
		elem.appendChild(createAnchorEscelem());//mark activeElement before `display='none'`,otherwise you cannot get back focus in some browser(s) (IE).
		document.body.firstElementChild.style.display='none';
		insBeforeFirstChild(document.body,elem);
		elem.lastChild.onclick=function(){search={};};
		elem.appendChild(createAnchorClelem());
		elem.lastChild.onclick=oc_separator;
		addrowsforverb(elem.lastChild)
		elem.appendChild(document.createElement('div'));
		elem.lastChild.innerHTML='Shortcuts: Enter nothing to add verb <i>Text</i> for searching all text. Enter " " to choose from top-level user-defined verbs. Enter "n" to add verb <i>Is A Text Note</i>. Enter "b" to add verb <i>Is A Bookmark</i>. etc.<br />Some Predefined Verbs: <i>URL</i> (in URL of bookmark), <i>Filename</i> (in filenames involved), <i>Desc</i> (in description involved), <i>*</i> for record using any user-defined verb.';
		search.h_submit=elem.appendChild(createAnchorClelem());
		elem.lastChild.text='Submit';
		elem.lastChild.onclick=function(){
			var sorvelem=this.parentNode.children[1];
			var bloborarr=new Uint8Array([tc.specialized_sql,11]);
			for(;;){
				if(sorvelem.nextSibling.tagName==='INPUT'){
					sorvelem.nextSibling.focus();
					alertmsg('There is unused row(s) for verb.');
					return;
				}
				var septosend=seseparatorreadytosubmit(sorvelem.textContent);
				if(septosend===undefined){
					sorvelem.focus();
					alertmsg('There is invalid separator(s).');
					return;
				}
				if(sorvelem.nextSibling.tagName==='DIV'){//optimize (micro) no need to check for first iteration
					bloborarr=new Blob([bloborarr,septosend,new Uint8Array(1)]);
					break;
				}
				bloborarr=new Blob([bloborarr,septosend,new Uint8Array([0,sorvelem.nextSibling.firstElementChild ? 0:1]),defwm.get(sorvelem.nextSibling)]);
				sorvelem=sorvelem.nextSibling.nextSibling;//can be the text input for `contains text` or verb
				if(sorvelem.nextSibling.tagName==='INPUT'){
					var enctext=deftextenc.encode(sorvelem.value);
					if(enctext.length<2){
						sorvelem.focus();
						alertmsg('Text for search is too short.');
						return;
					}
					if(enctext.length>textinlenlimit){sorvelem.focus();alertmsg('Input is too long');return;}
					bloborarr=new Blob([bloborarr,enctext,new Uint8Array(1)]);
					sorvelem=sorvelem.nextSibling;
				}
				sorvelem=sorvelem.nextSibling;//separator
			}
			postreqforarraybuffer(bloborarr,function(arrbuf){
				var idorerrmsg=new DataView(arrbuf,17);
				if(new Uint8Array(arrbuf,OFF,1)[0]){
					alertmsg(deftextdec.decode(idorerrmsg));
				}else{
					var elem=document.createElement('div');
					insBeforeFirstChild(elem,createAnchorEscelem());
					var u8b=new Uint8Array(arrbuf,OFF+1);
					var off=0;
					for(;off!==u8b.length;){
						var tcty=u8b[off];
						off++;
						let id=u8b.subarray(off,off+8);
						off+=8;
						switch(tcty){
						case tc.bookmark:
							var blen=getUint32LEFromBuffer(u8b,off);
							off+=4;
							var urlstring=deftextdec.decode(u8b.subarray(off,off+blen));
							elem.appendChild(createAnchorClelemWithText(urlstring)).insertAdjacentElement('afterbegin',document.createElement('br'));
							off+=blen;
							var blen=getUint32LEFromBuffer(u8b,off);
							off+=4;
							elem.lastChild.insertAdjacentText('afterbegin',deftextdec.decode(u8b.subarray(off,off+blen)));
							off+=blen;
							elem.lastChild.insertAdjacentHTML('afterbegin','<b style="font-size:70%">Bookmark </b>');
							elem.lastChild.onclick=function(){
								var cothis=this;
								reforbmpage(id,function(url,des){
									cothis.childNodes[1].nodeValue=des;
									cothis.lastChild.nodeValue=url;
								},cothis);
							};
							elem.lastChild.href=urlstring;
							break;
						case tc.text_note:
							var end=u8b.indexOf(0,off);
							var text=deftextdec.decode(u8b.subarray(off,end));
							off=end+1;
							elem.appendChild(createAnchorClelemWithText(text)).insertAdjacentHTML('afterbegin','<b style="font-size:70%">Text Note </b>');
							elem.lastChild.onclick=function(){
								var tnode=this.lastChild;
								crdivfornote(id,tnode.nodeValue,'',function(v){tnode.nodeValue=v;},this);
							};
							break;
						case tc.wordsense:
							var text='';
							for(;;){
								var blen=new DataView(u8b.buffer,u8b.byteOffset+off,4).getUint32(0,true);
								off+=4;
								if(blen===0xFFFFFFFF)break;
								text+=deftextdec.decode(u8b.subarray(off,off+blen))+', ';
								off+=blen;
							}
							elem.appendChild(createAnchorClelemWithText(text.slice(0,-2))).insertAdjacentHTML('afterbegin','<b style="font-size:70%">Word Group </b>');
							elem.lastChild.onclick=function(){
								var tnode=this.lastChild;
								showwordgrouppage(id,function(v){tnode.nodeValue=v;},this);
							};
							break;
						}
					}
					bodyhidefirstinsertfirstfocusonfirst(elem);
				}
			});
		};
		//elem.appendChild(document.createElement('div'));
		//elem.lastChild.textContent='Most used verbs: ';
		//elem.appendChild(document.createElement('div'));
		//undone (get tags) postreqforarraybuffer(bloborarr,function(arrbuf)
		//undone add most common tags as inline-block(s) to the div
		break;
	case 'bookmark':
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		appendLabelAndInput(elem,'Search URL and Description: ');
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!this.value)return;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'s'.charCodeAt(0)]),this.value]),function(arrbuf){
				var elem=document.createElement('div');
				insBeforeFirstChild(elem,createAnchorEscelem());
				for(var off=OFF;;){
					if(arrbuf.byteLength===off)break;
					let id=new Uint8Array(arrbuf,off,8);off+=8;
					var len=new DataView(arrbuf,off,4).getUint32(0,true);off+=4;
					var url=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
					var len=new DataView(arrbuf,off,4).getUint32(0,true);off+=4;
					var des=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
					var len=new DataView(arrbuf,off,4).getUint32(0,true);off+=4;
					var fnm=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
					elem.appendChild(appendbndiv(createAnchorClelem(),des,url)).onclick=function(){
						var cothis=this;
						function fntextupdate(url,des){
							cothis.firstChild.textContent=des;
							cothis.lastChild.textContent=url;
						}
						reforbmpage(id,fntextupdate,cothis);
					};
					elem.lastChild.href=url;
				}
				bodyhidefirstinsertfirstfocusonfirst(elem);
			});
		};
		elem.appendChild(createAnchorClelemWithText('Create Bookmark')).onclick=function(){
			showbmpage(undefined,[],'','','');
		};
		bodyhidefirstinsertfirstfocusonfirst(elem);
		break;
	case 'text_note':
		postreqforarraybuffer(new Uint8Array([tc.text_note,'p'.charCodeAt(0)]),function(arrbuf){
			var odv=new DataView(arrbuf,OFF);
			textnote.folderlist=[];
			var folderlist=textnote.folderlist;
			var editorlist=[];
			var off=readlistofdv(odv,0,folderlist);
			readlistofdvtillend(odv,off,editorlist);
			var elem=document.createElement('div');
			elem.innerHTML='<div class="fixedtip" >Do not rename or delete notes outside ECSBAS. It bewilders the database.</div>';
			var he=createAnchorEscelem();
			he.onclick=function(){textnote={};};
			insBeforeFirstChild(elem,he);
			appendLabelAndInput(elem,'Search all notes: ');
			elem.lastChild.lastChild.onkeydown=function(ke){
				if(ke.keyCode!==0x0d)return;
				searchallnotes(this.value);
			};
			//todo add row 'List recent modified notes'
			var he=document.createElement('label');
			he.textContent='Create Note With Filename: ';
			elem.appendChild(he);
			textnote.h_filenm=createTextInputPreventDefaultWhenEnterKeyDown();
			he.appendChild(textnote.h_filenm);
			textnote.h_filenm.onkeydown=okey_createnote;
			var he=document.createElement('label');
			he.textContent='Folder Name: ';
			elem.appendChild(he);
			textnote.h_foldernm=document.createElement('select');
			for(var folderle of folderlist){
				textnote.h_foldernm.add(new Option(deftextdec.decode(folderle)));
			}
			textnote.h_foldernm.add(new Option('<Specify a new folder name>'));
			he.appendChild(textnote.h_foldernm);
			textnote.h_foldernmtext=createTextInputPreventDefaultWhenEnterKeyDown();
			elem.appendChild(textnote.h_foldernmtext);
			textnote.h_foldernmtext.style.display='none';
			textnote.h_foldernm.onchange=function(){
				var selfolder=folderlist[this.selectedIndex];
				if(selfolder){
					var bloborarr=new Uint8Array([tc.specialized_sql,0]);
					postreqforarraybuffer(new Blob([bloborarr,selfolder]),()=>{});
					textnote.h_foldernmtext.style.display='none';
				}else{
					textnote.h_foldernmtext.style.display=''; textnote.h_foldernmtext.focus();
				}
			};
			var he=document.createElement('label');
			he.textContent='Editor for Opening Notes: ';
			elem.appendChild(he);
			var hse=document.createElement('select');
			for(var editorle of editorlist){
				hse.add(new Option(deftextdec.decode(editorle)));
				defwm.set(hse.lastChild,editorle);
			}
			hse.onchange=function(){
				var seleditor=defwm.get(firstSelectedOptionIn(this));
				var bloborarr=new Uint8Array([tc.specialized_sql,1]);
				postreqforarraybuffer(new Blob([bloborarr,seleditor]),alertifarrbufneoff);
			};
			he.appendChild(hse);
			var he=createAnchorClelem();
			he.text='Add Command for Editor';
			he.onclick=function(){
				promptmsg('Command to open note file ',function(cmd){
					if(cmd){
						postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,2]),cmd]),function(arrbuf){
							if(arrbuf.byteLength===OFF){
								hse.add(new Option(cmd));
								defwm.set(hse.lastChild,cmd);
								fnforesc();
							}else{
								alertmsg('Command already exists');
							}
						});
					}
				});
			};
			elem.appendChild(he);
			var he=createAnchorClelem();
			he.text='Remove Commands for Editor';
			he.onclick=function(){
				var elem=document.createElement('div');
				insBeforeFirstChild(elem,createAnchorEscelem());
				var hte=document.createElement('table');
				for(let hseo of hse.options){
					var htre=makeClelem(hte.insertRow());
					htre.insertCell().textContent=hseo.text;
					htre.insertCell().textContent='\u274c';
					htre.lastChild.style.width='1px';
					htre.onclick=function(){
						var htre=this;
						postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,3]),defwm.get(hseo)]),function(arrbuf){
							hse.remove(htre.rowIndex);
							removeFromParentNode(htre);
							var odv=new DataView(arrbuf,OFF);
							if(odv.byteLength){
								var maintextedit=deftextdec.decode(odv);
								for(let hseo of hse.options){
									if(hseo.text===maintextedit){
										hseo.selected=true;
										return;
									}
								}
								throw new Error("Main editor not found in current tab. Unexpected newly added editor?");
							}
						});
					};
				}
				elem.appendChild(hte);
				bodyhidefirstinsertfirstfocusonfirst(elem);
			};
			elem.appendChild(he);
			bodyhidefirstinsertfirstfocusonfirst(elem);
		});
		break;
	case 'verbmgmt':
		var elem=document.createElement('div');
		elem.appendChild(createAnchorEscelem());
		elem.appendChild(document.createElement('label'));
		elem.lastChild.textContent='Search description of SA verbs: ';
		elem.lastChild.appendChild(createTextInputPreventDefaultWhenEnterKeyDown());
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!this.value)return;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,5]),this.value]),listsaverbs);
		};
		elem.appendChild(createAnchorClelem()).text='List top-level SA verbs (the verbs that are not entailing any verbs)';
		elem.lastChild.onclick=function(){
			postreqforarraybuffer(new Uint8Array([tc.specialized_sql,4]),listsaverbs);
		};
		//"is-a text note", "is-a bookmark" are included in the listed top-level verbs, though they are not modifiable verbs. When a verb X entails "is-a text note", it means currently the X is not used anywhere except text note records. //but wait, that's wrong!! e.g. user add a bookmark and attempt to attach verb X, but X should be invisible bc "is-a text note" and "is-a bookmark" is in mutex group! //it seems this is a classic example of difference between immanent and adventitious verbs!
		//
		//todo user can select one or more among listed/found verbs as search criteria combined with "Search verb description"
		elem.appendChild(createAnchorClelem()).text='Create an SA verb';
		elem.lastChild.onclick=promptmsg.bind(undefined,'Description of new SA verb: ',function(desc){
			if(!desc)return;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,12]),desc]),function(arrbuf){
				var idorerrmsg=new Uint8Array(arrbuf,17);
				if(new Uint8Array(arrbuf,OFF,1)[0]){
					alertmsg(deftextdec.decode(idorerrmsg));
				}else{
					fnforesc();
					showverbpage(desc,idorerrmsg);
				}
			});
		});
		bodyhidefirstinsertfirstfocusonfirst(elem);
		break;
	case 'wordsense':
		var elem=document.createElement('div');
		elem.appendChild(createAnchorEscelem());
		appendLabelAndInput(elem,'Search for word groups: ');
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!this.value)return;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.wordsense,'s'.charCodeAt(0)]),this.value]),function(arrbuf){
				var elem=document.createElement('div');
				insBeforeFirstChild(elem,createAnchorEscelem());
				var off=OFF;
				if(off!==arrbuf.byteLength){
					outer:for(;;){
						var id=new Uint8Array(arrbuf,off,8);
						off+=8;
						var len=new DataView(arrbuf,off,4).getUint32(0,true);
						off+=4;
						let seres=elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len))));
						seres.onclick=showwordgrouppage.bind(undefined,id,function(v){seres.text=v;},elem.lastChild);
						for(;;){
							off+=len;
							if(off===arrbuf.byteLength)break outer;
							len=new DataView(arrbuf,off,4).getUint32(0,true);
							off+=4;
							if(len===0xFFFFFFFF)break;
							var str=deftextdec.decode(new DataView(arrbuf,off,len));
							elem.lastChild.text+=', '+str;
						}
					}
				}
				bodyhidefirstinsertfirstfocusonfirst(elem);
			});
		};
		elem.appendChild(createAnchorClelem()).text='Make a word group';
		elem.lastChild.onclick=function(){
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			appendLabelAndInput(elem,'Add word: ');
			elem.lastChild.lastChild.onkeydown=function(ke){
				if(ke.keyCode!==0x0d)return;
				addwordelemtodiv(this.parentNode,this.value);
				this.value='';
			};
			elem.appendChild(createAnchorClelem()).text='Save';
			elem.lastChild.onclick=function(){
				var bloborarr=new Uint8Array([tc.wordsense,'i'.charCodeAt(0)]);
				var wordelem=this.previousSibling.previousSibling;
				if(!wordelem.classList.contains('clelem')){alertmsg('Need to add word before saving.');return;}
				do{
					bloborarr=new Blob([bloborarr,wordelem.text,new Uint8Array(1)]);
					wordelem=wordelem.previousSibling;
				}while(wordelem.classList.contains('clelem'));
				postreqforarraybuffer(bloborarr,function(arrbuf){
					var idorerrmsg=new DataView(arrbuf,17);
					if(new Uint8Array(arrbuf,OFF,1)[0]){
						alertmsg(deftextdec.decode(idorerrmsg));
					}else{
						fnforesc();
						showwordgrouppage(idorerrmsg);
					}
				});
			};
			bodyhidefirstinsertfirstfocusonfirst(elem);
		};
		bodyhidefirstinsertfirstfocusonfirst(elem);
		break;
	}
}
function addwordelemtodiv(l,v){
	l.parentNode.insertBefore(createAnchorClelemWithText(v),l).onclick=function(){
		this.nextSibling.focus();//when label gets focus, it focuses on its associated labelable element?
		removeFromParentNode(this);
	};
}
function showwordgrouppage(id,fntextupdate,rowforremoval){
	postreqforarraybuffer(new Blob([new Uint8Array([tc.wordsense,'S'.charCodeAt(0)]),id]),function(arrbuf){
		if(arrbuf.byteLength===OFF) {alertmsg('Word group is already deleted.');}else{
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem(''));
			appendLabelAndInput(elem,'Add word: ');
			elem.lastChild.lastChild.onkeydown=function(ke){
				if(ke.keyCode!==0x0d)return;
				addwordelemtodiv(this.parentNode,this.value);
				this.value='';
			};
			var off=OFF;do{
				var len=new DataView(arrbuf,off,4).getUint32(0,true);
				off+=4;
				addwordelemtodiv(elem.lastChild,deftextdec.decode(new DataView(arrbuf,off,len)));
				off+=len;
			}while(arrbuf.byteLength!==off);
			elem.appendChild(createAnchorClelem()).text='Save';
			elem.lastChild.onclick=function(){
				var textforupdate='';
				var bloborarr=new Blob([new Uint8Array([tc.wordsense,'u'.charCodeAt(0)]),id]);
				var wordelem=this.previousSibling.previousSibling;
				if(!wordelem.classList.contains('clelem')){alertmsg('Need to add word before saving.');return;}
				do{
					textforupdate+=wordelem.text+', ';
					bloborarr=new Blob([bloborarr,wordelem.text,new Uint8Array(1)]);
					wordelem=wordelem.previousSibling;
				}while(wordelem.classList.contains('clelem'));
				postreqforarraybuffer(bloborarr,alertifarrbufneoff,function(){
					if(fntextupdate)fntextupdate(textforupdate.slice(0,-2));
					showtrmsg('Successfully saved.');
				});
			};
			elem.appendChild(createAnchorClelemWithText('Delete')).onclick=function(){
				confirmmsg('Delete the word group?',function(){
					fnforesc();
					postreqforarraybuffer(new Blob([new Uint8Array([tc.wordsense,'d'.charCodeAt(0)]),id]),function(){
						if(rowforremoval)removeFromParentNode(rowforremoval);
						fnforesc();
					});
				});
			};
			bodyhidefirstinsertfirstfocusonfirst(elem);
		}
	});
}
function appendLabelAndInput(elem,lbltext){
	elem.appendChild(document.createElement('label')).textContent=lbltext;
	elem.lastChild.appendChild(createTextInputPreventDefaultWhenEnterKeyDown());
}
function seremoverownsepabove(useverbin){
	if(useverbin.previousSibling.tagName==='INPUT'){
		removeFromParentNode(useverbin.previousSibling);
		removeFromParentNode(useverbin.previousSibling);
	}else if(useverbin.style.display){
		removeFromParentNode(useverbin.previousSibling);
	}
	fnforesc();
	if(useverbin.previousSibling.previousSibling.classList.contains('escelem')&&useverbin.nextSibling.nextSibling.tagName==='DIV'){
		useverbin.style.display='';
		useverbin.focus();
	}else{
		useverbin.nextSibling.focus();
		removeFromParentNode(useverbin.previousSibling);
		removeFromParentNode(useverbin);
	}
}
function seremoverownsepbelow(useverbin){
	if(useverbin.previousSibling.tagName==='INPUT'){
		removeFromParentNode(useverbin.previousSibling);
		removeFromParentNode(useverbin.previousSibling);
	}else if(useverbin.style.display){
		removeFromParentNode(useverbin.previousSibling);
	}
	fnforesc();
	if(useverbin.previousSibling.previousSibling.classList.contains('escelem')&&useverbin.nextSibling.nextSibling.tagName==='DIV'){
		useverbin.style.display='';
		useverbin.focus();
	}else{
		useverbin.previousSibling.focus();
		removeFromParentNode(useverbin.nextSibling);
		removeFromParentNode(useverbin);
	}
}
function oc_chverb(useverbin){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem());
	elem.appendChild(createAnchorClelem()).text='Change verb';
	elem.lastChild.onclick=function(){
		useverbin.style.display='';
		if(useverbin.previousSibling.tagName==='INPUT')
			removeFromParentNode(useverbin.previousSibling);
		removeFromParentNode(useverbin.previousSibling);
		fnforesc();
		useverbin.focus();
	};
	elem.appendChild(createAnchorClelem()).text='Remove the row (and the separator above)';
	elem.lastChild.onclick=seremoverownsepabove.bind(undefined,useverbin);
	elem.appendChild(createAnchorClelem()).text='Remove the row (and the separator below)';
	elem.lastChild.onclick=seremoverownsepbelow.bind(undefined,useverbin);
	elem.appendChild(createAnchorClelem()).text='NOT';
	var cothis=this;
	elem.lastChild.onclick=function(){
		if(cothis.firstElementChild){
			cothis.removeChild(cothis.firstElementChild);
		}else{
			insBeforeFirstChild(cothis,document.createElement('i'));
			cothis.firstElementChild.style.fontSize='70%';
			cothis.firstElementChild.textContent='Negation ';
		}
		fnforesc();
	};
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function listverbsforsel(arrbuf,useverbin){
	var off=OFF;
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem());
	for(;arrbuf.byteLength>off;){
		let id=new Uint8Array(arrbuf,off,8);
		off+=8;
		var len=new DataView(arrbuf,off,4).getUint32(0,true);
		off+=4;
		elem.appendChild(createAnchorClelem()).text=deftextdec.decode(new DataView(arrbuf,off,len));
		off+=len;
		elem.lastChild.onclick=function(){
			useverbin.style.display='none';
			useverbin.parentNode.insertBefore(this,useverbin);
			defwm.set(this,id);
			this.onclick=oc_chverb.bind(this,useverbin);
			fnforesc();
			switch(this.text){//? is comparing string safe? even for invalid code units?
			case 'Text':
			case 'URL':
			case 'Filename':
			case 'Desc':
				useverbin.parentNode.insertBefore(createTextInputPreventDefaultWhenEnterKeyDown(),useverbin).onkeydown=function(ke){
					if(ke.keyCode!==0x0d)return;
					search.h_submit.click();
				};
			}
			useverbin.previousSibling.focus();
		};
	}
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function prev_elem_is_clelem(nod){
	return nod.previousElementSibling.classList.contains('clelem');
}
function next_elem_is_clelem(nod){
	return nod.nextElementSibling && nod.nextElementSibling.classList.contains('clelem');
}
function oc_verbtreenode(){
	var verbtreenode=this;
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem());
	if(prev_elem_is_clelem(verbtreenode))elem.appendChild(createAnchorClelemWithText('Show entailed verbs (remove verbs above from the tree)')).onclick=function(){
		fnforesc();
		do{
			removeFromParentNode(verbtreenode.previousSibling);
			removeFromParentNode(verbtreenode.previousSibling);
		}while(prev_elem_is_clelem(verbtreenode));
		sendgethigherverbs(defwm.get(verbtreenode));
	};
	if(next_elem_is_clelem(verbtreenode))elem.appendChild(createAnchorClelemWithText('Show the verbs entailing this verb (remove verbs below from the tree)')).onclick=function(){
		fnforesc();
		do{
			removeFromParentNode(verbtreenode.nextSibling);
			removeFromParentNode(verbtreenode.nextSibling);
		}while(next_elem_is_clelem(verbtreenode));
		sendgetlowerverbs(defwm.get(verbtreenode));
	};
	appendLabelAndInput(elem,'Associate with verb (will be placed above):');
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!this.value)return;
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,5]),this.value]),function(arrbuf){
			fnforesc();
			var off=OFF;
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			for(;arrbuf.byteLength>off;){
				let id=new Uint8Array(arrbuf,off,8);
				off+=8;
				var len=new DataView(arrbuf,off,4).getUint32(0,true);
				off+=4;
				elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len))));
				off+=len;
				elem.lastChild.onclick=function(){
					var cothistext=this.text;
					postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,15]),defwm.get(verbtreenode),id]),alertifarrbufneoff,function(){
						fnforesc();
						while(prev_elem_is_clelem(verbtreenode)){
							removeFromParentNode(verbtreenode.previousSibling);
							removeFromParentNode(verbtreenode.previousSibling);
						}
						defwm.set(verbtreenode.insertAdjacentElement('beforebegin',createAnchorClelemWithText(cothistext)),id);
						verbtreenode.previousSibling.onclick=oc_verbtreenode;
						verbtreenode.insertAdjacentText('beforebegin','▲');
						sendgethigherverbs(id);
					});
				};
			}
			bodyhidefirstinsertfirstfocusonfirst(elem);
		});
	};
	appendLabelAndInput(elem,'Associate with verb (will be placed below):');
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!this.value)return;
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,5]),this.value]),function(arrbuf){
			fnforesc();
			var off=OFF;
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			for(;arrbuf.byteLength>off;){
				let id=new Uint8Array(arrbuf,off,8);
				off+=8;
				var len=new DataView(arrbuf,off,4).getUint32(0,true);
				off+=4;
				elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len))));
				off+=len;
				elem.lastChild.onclick=function(){
					var cothistext=this.text;
					postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,15]),id,defwm.get(verbtreenode)]),alertifarrbufneoff,function(){
						fnforesc();
						while(next_elem_is_clelem(verbtreenode)){
							removeFromParentNode(verbtreenode.nextSibling);
							removeFromParentNode(verbtreenode.nextSibling);
						}
						defwm.set(verbtreenode.insertAdjacentElement('afterend',createAnchorClelemWithText(cothistext)),id);
						verbtreenode.nextSibling.onclick=oc_verbtreenode;
						verbtreenode.insertAdjacentText('afterend','▲');
						sendgetlowerverbs(id);
					});
				};
			}
			bodyhidefirstinsertfirstfocusonfirst(elem);
		});
	};
	if(prev_elem_is_clelem(verbtreenode))elem.appendChild(createAnchorClelemWithText('Disassociate from the verb above')).onclick=function(){
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,16]),defwm.get(verbtreenode),defwm.get(verbtreenode.previousElementSibling)]),function(){
			fnforesc();
			while(prev_elem_is_clelem(verbtreenode)){
				removeFromParentNode(verbtreenode.previousSibling);
				removeFromParentNode(verbtreenode.previousSibling);
			}
			sendgethigherverbs(defwm.get(verbtreenode));
		});
	};
	if(next_elem_is_clelem(verbtreenode))elem.appendChild(createAnchorClelemWithText('Disassociate from the verb below')).onclick=function(){
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,16]),defwm.get(verbtreenode.nextElementSibling),defwm.get(verbtreenode)]),function(){
			fnforesc();
			while(next_elem_is_clelem(verbtreenode)){
				removeFromParentNode(verbtreenode.nextSibling);
				removeFromParentNode(verbtreenode.nextSibling);
			}
			sendgetlowerverbs(defwm.get(verbtreenode));
		});
	};
	elem.appendChild(createAnchorClelemWithText('Edit this verb')).onclick=function(){
		fnforesc();
		fnforesc();//close tree
		fnforesc();//close verb page
		showverbpage(verbtreenode.text,defwm.get(verbtreenode));
	};
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function sendgethigherverbs(id){
	postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,13]),id]),function(arrbuf){
		var u8b=new Uint8Array(arrbuf,OFF);
		var listoftuhigher=[];
		readlistofidstrtillend(u8b,listoftuhigher);
		inserthigheribcl(listoftuhigher);
	});
}
function sendgetlowerverbs(id){
	postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,14]),id]),function(arrbuf){
		var u8b=new Uint8Array(arrbuf,OFF);
		var listoftulower=[];
		readlistofidstrtillend(u8b,listoftulower);
		insertloweribcl(listoftulower);
	});
}
function inserthigheribcl(listoftu){
	var divtocle=document.body.firstChild.children[1];
	divtocle.innerHTML='';
	for(let vhigher of listoftu){
		divtocle.appendChild(createAnchorIbclelemWithText(vhigher.s)).onclick=function(){
			defwm.set(divtocle.insertAdjacentElement('afterend',createAnchorClelemWithText(this.text)),vhigher.i);
			divtocle.nextSibling.onclick=oc_verbtreenode;
			divtocle.nextSibling.focus();
			divtocle.insertAdjacentText('afterend','▲');
			sendgethigherverbs(vhigher.i);
		};
	}
}
function insertloweribcl(listoftu){
	var divtocle=document.body.firstChild.lastChild;
	divtocle.innerHTML='';
	for(let vlower of listoftu){
		divtocle.appendChild(createAnchorIbclelemWithText(vlower.s)).onclick=function(){
			defwm.set(divtocle.insertAdjacentElement('beforebegin',createAnchorClelemWithText(this.text)),vlower.i);
			divtocle.previousSibling.onclick=oc_verbtreenode;
			divtocle.previousSibling.focus();
			divtocle.insertAdjacentText('beforebegin','▲');
			sendgetlowerverbs(vlower.i);
		};
	}
}
function showverbpage(desc,id){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem(''));
	appendLabelAndInput(elem,'Edit description of verb: ');
	elem.lastChild.lastChild.value=desc;
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		var newdesc=this.value;
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,6]),id,newdesc]),alertifarrbufneoff,function(){
			for(var snode of document.body.querySelectorAll("[data-listsaverbs='"+id.toString()+"']"))
				snode.textContent=newdesc;
			desc=newdesc;//wo setting this, showing tree will show old desc
			showtrmsg('Successfully saved.');
		});
	};
	elem.appendChild(createAnchorClelem());
	elem.lastChild.text='Show the tree of verbs';
	elem.lastChild.onclick=function(){
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,7]),id]),function(arrbuf){
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			elem.appendChild(document.createElement('div'));
			var u8b=new Uint8Array(arrbuf,OFF);
			var listoftuhigher=[];
			var off=readlistofidstr(u8b,listoftuhigher);
			var listoftulower=[];
			readlistofidstrtillend(u8b.subarray(off),listoftulower);
			bodyhidefirstinsertfirstfocusonfirst(elem);
			elem.appendChild(document.createTextNode('▲'));//\u25b2
			elem.appendChild(createAnchorClelemWithText(desc)).focus();
			defwm.set(elem.lastChild,id);
			elem.lastChild.onclick=oc_verbtreenode;
			elem.appendChild(document.createTextNode('▲'));
			elem.appendChild(document.createElement('div'));
			inserthigheribcl(listoftuhigher);
			insertloweribcl(listoftulower);
		});
	};
	elem.appendChild(createAnchorClelem()).text='Mutually exclusive verb groups';
	elem.lastChild.onclick=function(){
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		elem.appendChild(createAnchorClelem()).text='List mutually exclusive verb groups containing this verb';
		elem.lastChild.onclick=function(){
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,8]),id]),function(arrbuf){
				var elem=document.createElement('div');
				insBeforeFirstChild(elem,createAnchorEscelem());
				var u8b=new Uint8Array(arrbuf,OFF);
				var off=0;
				if(off!==u8b.length){
					outer:for(;;){
						let id=u8b.subarray(off,off+8);
						off+=8;
						elem.appendChild(createAnchorClelem()).text=desc;
						elem.lastChild.onclick=function(){
							var cothis=this;
							postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,26]),id]),function(arrbuf){
								if(arrbuf.byteLength===OFF){
									alertmsg('Record not found.');
								}else{
									var listoftu=[];
									var newelemtext='';
									for(var off=OFF;;){
										var vid=new Uint8Array(arrbuf,off,8);
										off+=8;
										var len=new DataView(arrbuf,off,4).getUint32(0,true);
										off+=4;
										var vdes=deftextdec.decode(new DataView(arrbuf,off,len));
										listoftu.push({i:vid,s:vdes});
										newelemtext+=vdes+', ';
										off+=len;
										if(off===arrbuf.byteLength)break;
									}
									cothis.text=newelemtext.slice(0,-2);
									showmutexpage(id,listoftu,cothis);
								}
							});
						};
						for(;;){
							var end=u8b.indexOf(0,off);
							var vdesc=deftextdec.decode(u8b.subarray(off,end));
							elem.lastChild.text+=', '+vdesc;
							off=end+1;
							if(off===u8b.length) break outer;
							if(!u8b[off])break;
							++off;
						}
						++off;
					}
				}
				bodyhidefirstinsertfirstfocusonfirst(elem);
			});
		};
		elem.appendChild(createAnchorClelem()).text='Make new group';
		elem.lastChild.onclick=function(){
			showmutexpage(undefined,[{i:id,s:desc}]);
		};
		bodyhidefirstinsertfirstfocusonfirst(elem);
	};
	elem.appendChild(createAnchorClelem());
	elem.lastChild.text='Delete';
	elem.lastChild.onclick=function(){
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,9]),id]),alertifarrbufneoff,function(arrbuf){
			for(var snode of document.body.querySelectorAll("[data-listsaverbs='"+id.toString()+"']"))
				removeFromParentNode(snode);
			fnforesc();
		});
	};
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function showmutexpage(meid,listoftu,rowforremoval){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem(''));
	appendLabelAndInput(elem,'Search description to add SA verbs: ');
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!this.value)return;
		postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,5]),this.value]),function(arrbuf){
			var off=OFF;
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			for(;arrbuf.byteLength>off;){
				let id=new Uint8Array(arrbuf,off,8);
				off+=8;
				var len=new DataView(arrbuf,off,4).getUint32(0,true);
				off+=4;
				elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len)))).onclick=function(){
					var velem=selem.nextSibling;
					var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,23]),id]);
					for(var i=0;velem;++i){
						bloborarr=new Blob([bloborarr,defwm.get(velem)]);
						velem=velem.nextSibling;
					}
					var textofthis=this.text;
					function addverbtogrolist(){
						selem.insertAdjacentElement('afterend',createAnchorIbclelemWithText(textofthis)).onclick=function(){this.previousSibling.focus();removeFromParentNode(this);};
						defwm.set(selem.nextSibling,id);
						fnforesc();
					}
					if(i){
						postreqforarraybuffer(bloborarr,function(arrbuf){
							if(arrbuf.byteLength===OFF){
								addverbtogrolist();
							}else{
								alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
							}
						});
					}else addverbtogrolist();
				};
				off+=len;
			}
			bodyhidefirstinsertfirstfocusonfirst(elem);
		});
	};
	if(meid){
		elem.appendChild(createAnchorClelemWithText('Delete')).onclick=function(){
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,25]),meid]),function(arrbuf){
				if(rowforremoval)removeFromParentNode(rowforremoval);
				fnforesc();
			});
		};
	}
	elem.appendChild(createAnchorClelemWithText('Save')).onclick=function(){
		var velem=selem.nextSibling;
		if(meid)var comeid=meid;
		else var comeid=new Uint8Array(8);
		var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,24]),comeid]);
		var newelemtext='';
		for(var i=0;velem;++i){
			bloborarr=new Blob([bloborarr,defwm.get(velem)]);
			newelemtext+=velem.text+', ';
			velem=velem.nextSibling;
		}
		if(i<2){alertmsg('A group needs at least two verbs.');return;}
		postreqforarraybuffer(bloborarr,function(arrbuf){
			if(arrbuf.byteLength===OFF){
				if(meid){
					rowforremoval.text=newelemtext.slice(0,-2);
					showtrmsg('Successfully saved.');
				}else{fnforesc();}
			}else{
				alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
			}
		});
	};
	var selem=elem.lastChild;
	for(var tu of listoftu){
		elem.appendChild(createAnchorIbclelemWithText(tu.s)).onclick=function(){this.previousSibling.focus();removeFromParentNode(this);};
		defwm.set(elem.lastChild,tu.i);
	}
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function listsaverbs(arrbuf){
	var off=OFF;
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem());
	for(;arrbuf.byteLength>off;){
		let id=new Uint8Array(arrbuf,off,8);
		off+=8;
		var len=new DataView(arrbuf,off,4).getUint32(0,true);
		off+=4;
		elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len))));
		off+=len;
		elem.lastChild.setAttribute('data-listsaverbs',id.toString());
		elem.lastChild.onclick=function(){showverbpage(this.text,id);}
	}
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function searchallnotes(setext){
	var enctext=deftextenc.encode(setext);
	if(enctext.length<2){alertmsg('Input is too short');return;}
	if(enctext.length>textinlenlimit){alertmsg('Input is too long');return;}
	postreqforarraybuffer(new Blob([new Uint8Array([tc.text_note,'s'.charCodeAt(0)]),enctext]),function(arrbuf){
		//textnote.searchresults=[];
		var off=0;
		var u8b=new Uint8Array(arrbuf,OFF);
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		for(;u8b.length>off;){
			let syno=u8b.subarray(off,off+SYNOPSISLEN);
			syno=deftextdec.decode(syno.subarray(0,indexOfLastNotEq(syno,0)+1));
			off+=SYNOPSISLEN;
			let id=u8b.subarray(off,off+8);
			off+=8;
			var coff=off;
			off=u8b.indexOf(0,off);
			var dirnfilenm=deftextdec.decode(u8b.subarray(coff,off));
			++off;
			//textnote.searchresults.push(id);
			let seresultnote=createAnchorClelem();
			elem.appendChild(seresultnote);
			appendbndiv(seresultnote,dirnfilenm,syno);
			seresultnote.onclick=function(){crdivfornote(id,seresultnote.firstChild.textContent,syno,function(v){
				seresultnote.firstChild.textContent=v;
			},seresultnote);};
		}
		bodyhidefirstinsertfirstfocusonfirst(elem);
	});
}
function alertifarrbufneoff(arrbuf,caseok){
	if(arrbuf.byteLength!==OFF)
		alertmsg(deftextdec.decode(new DataView(arrbuf,OFF)));
	else if(caseok)
		caseok();
}
function appendbndiv(elem,tc0,tc1){
	elem.appendChild(document.createElement('b'));
	elem.lastChild.textContent=tc0;
	elem.appendChild(document.createElement('div'));
	elem.lastChild.textContent=tc1;
	return elem;
}
function okey_createnote(ke){
	if(ke.keyCode!==0x0d)return;
	if(textnote.h_foldernm.length===textnote.h_foldernm.selectedIndex+1){
		if(!foldernmreg.test(textnote.h_foldernmtext.value)){alertfoldernmnotallowed();return;}
		var foldernm=textnote.h_foldernmtext.value;
		var strforchcode='I';
	}else{
		var strforchcode='i';
	}
	var filenm=textnote.h_filenm.value;
	if(!filenmreg.test(filenm)){alertfilenmnotallowed();return;}
	var bloborarr=new Uint8Array([tc.text_note,strforchcode.charCodeAt(0)]);
	if(typeof foldernm !== 'undefined'){
		bloborarr=new Blob([bloborarr,foldernm,new Uint8Array(1)]);
	}
	postreqforarraybuffer(new Blob([bloborarr,filenm]),function(arrbuf){
		var idorerrmsg=new DataView(arrbuf,17);
		if(new Uint8Array(arrbuf,OFF,1)[0]){
			alertmsg(deftextdec.decode(idorerrmsg));
		}else{
			if(typeof foldernm !== 'undefined'){
				addoelem:{
					for(var oelem of textnote.h_foldernm.options)
						if(oelem.text===foldernm) break addoelem;
					textnote.folderlist.push(foldernm);
					textnote.h_foldernm.add(new Option(foldernm),textnote.h_foldernm.length-1);
				}
			}else
				foldernm=textnote.h_foldernm.value;
			crdivfornote(idorerrmsg,foldernm+'/'+filenm,'');
		}
	});
}
function crdivfornote(sid,dirnfilenm,syno,fntextupdate,rowforremoval){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem(''));
	var seresnoteti=document.createElement('div');
	elem.appendChild(seresnoteti);
	appendbndiv(seresnoteti,dirnfilenm,syno);
	seresnoteti.className='fixedtip';
	elem.appendChild(createAnchorClelem());
	elem.lastChild.text='Edit';
	elem.lastChild.onclick=function(){
		postreqforarraybuffer(new Blob([new Uint8Array([tc.text_note,'e'.charCodeAt(0)]),sid]),alertifarrbufneoff);
	};
	elem.appendChild(createAnchorClelem());
	elem.lastChild.text='Rename (Move)';
	elem.lastChild.onclick=function(){
		postreqforarraybuffer(new Uint8Array([tc.text_note,'f'.charCodeAt(0)]),function(arrbuf){
			var u8b=new Uint8Array(arrbuf,OFF);
			var folderlist=decodeutf8strs(u8b);
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			elem.appendChild(document.createElement('div'));
			elem.lastChild.innerHTML=seresnoteti.innerHTML;
			elem.lastChild.className='fixedtip';
			var he=document.createElement('label');
			he.textContent='Folder Name: ';
			var selelem=document.createElement('select');
			he.appendChild(selelem);
			for(var folderle of folderlist)
				selelem.add(new Option(folderle));
			selelem.add(new Option('<Specify a new folder name>'));
			var dirnfilenm=seresnoteti.firstChild.textContent;
			selelem.value=dirnfilenm.slice(0,dirnfilenm.indexOf('/'));
			elem.appendChild(he);
			var foldernmtext=createTextInputPreventDefaultWhenEnterKeyDown();
			elem.appendChild(foldernmtext);
			foldernmtext.style.display='none';
			selelem.onchange=function(){
				foldernmtext.style.display=selelem.selectedIndex+1===selelem.length?'':'none';
			};
			var he=document.createElement('label');
			he.textContent='Filename: ';
			var textin=createTextInputPreventDefaultWhenEnterKeyDown();
			he.appendChild(textin);
			textin.value=dirnfilenm.slice(dirnfilenm.indexOf('/')+1);
			elem.appendChild(he);
			elem.appendChild(createAnchorClelem());
			elem.lastChild.text='Submit';
			elem.lastChild.onclick=function(){
				if(!filenmreg.test(textin.value)){alertfilenmnotallowed();return;}
				if(selelem.selectedIndex+1===selelem.length){
					if(!foldernmreg.test(foldernmtext.value)){alertfoldernmnotallowed();return;}
					var selfolder=foldernmtext.value;
				}else{
					var selfolder=selelem.value;
				}
				postreqforarraybuffer(new Blob([new Uint8Array([tc.text_note,'u'.charCodeAt(0)]),sid,selfolder,new Uint8Array(1),textin.value]),alertifarrbufneoff,function(){
					fnforesc();
					seresnoteti.firstChild.textContent=selfolder+'/'+textin.value;
					if(fntextupdate)fntextupdate(seresnoteti.firstChild.textContent);
				});
			};
			bodyhidefirstinsertfirstfocusonfirst(elem);
		});
	};
	elem.appendChild(createAnchorClelemWithText('Delete')).onclick=function(){
		confirmmsg('Delete the text note?',function(){
			fnforesc();
			postreqforarraybuffer(new Blob([new Uint8Array([tc.text_note,'d'.charCodeAt(0)]),sid]),alertifarrbufneoff,function(){
				if(rowforremoval)removeFromParentNode(rowforremoval);
				fnforesc();
			});
		});
	};
	elem.appendChild(createAnchorClelemWithText('Attach or detach SA verbs')).onclick=function(){
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		elem.appendChild(createAnchorClelemWithText('Applicable SA verbs (top)')).onclick=function(){
			var bloborarr=new Uint8Array([tc.specialized_sql,18]);
			for(var ibcle of getcurrentdiv().getElementsByClassName('ibclelem'))
				bloborarr=new Blob([bloborarr,defwm.get(ibcle)]);
			if(Uint8Array.prototype.isPrototypeOf(bloborarr)){
				bloborarr[1]=4;
				postreqforarraybuffer(bloborarr,function(arrbuf){
					adddivoflistofverbstoattach(arrbuf,OFF);
				});
			}else{
				postreqforarraybuffer(bloborarr,function(arrbuf){
					if(new Uint8Array(arrbuf,OFF,1)[0]){
						alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
					}else{
						adddivoflistofverbstoattach(arrbuf,OFF+1);
					}
				});
			}
		};
		appendLabelAndInput(elem,'Search description of SA verbs: ');
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!this.value)return;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,5]),this.value]),function(arrbuf){
				adddivoflistofverbstoattach(arrbuf,OFF);
			});
		};
		//undone appendLabelAndInput(elem,'Attach/Create verb with exact description: ');
		elem.appendChild(createAnchorClelemWithText('Save changes')).onclick=function(){
			var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,19]),sid]);
			for(var ibcle of getcurrentdiv().getElementsByClassName('ibclelem'))
				bloborarr=new Blob([bloborarr,defwm.get(ibcle)]);
			postreqforarraybuffer(bloborarr,function(arrbuf){
				if(arrbuf.byteLength!==OFF)
					alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
				else{
					refreshsaverbsreadonly(sid,getlowerdiv());
					fnforesc();
				}
			});
		};
		bodyhidefirstinsertfirstfocusonfirst(elem);
		getsaverbsattachedtosense(sid,function(listoftu){
			for(var tu of listoftu)
				elem.appendChild(crverbibclelem(tu.i,tu.s));
		});
	};
	//todo
	//elem.appendChild(createAnchorClelemWithText('View all implicit verbs')).onclick=function(){
	//};
	refreshsaverbsreadonly(sid,elem);
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function adddivoflistofverbstoattach(arrbuf,off){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem());
	for(;arrbuf.byteLength>off;){
		let id=new Uint8Array(arrbuf,off,8);
		off+=8;
		var len=new DataView(arrbuf,off,4).getUint32(0,true);
		off+=4;
		elem.appendChild(createAnchorClelemWithText(deftextdec.decode(new DataView(arrbuf,off,len)))).onclick=function(){
			var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,17]),id]);
			for(var ibcle of getlowerdiv().getElementsByClassName('ibclelem'))
				bloborarr=new Blob([bloborarr,defwm.get(ibcle)]);
			var cothistext=this.text;
			postreqforarraybuffer(bloborarr,function(arrbuf){
				if(arrbuf.byteLength!==OFF)
					alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
				else{
					fnforesc();
					getcurrentdiv().appendChild(crverbibclelem(id,cothistext));
				}
			});
		};
		off+=len;
	}
	bodyhidefirstinsertfirstfocusonfirst(elem);
}
function crverbibclelem(i,s){
	var ve=createAnchorIbclelemWithText(s);
	defwm.set(ve,i);
	ve.onclick=function(){
		var cothis=this;
		var elem=document.createElement('div');
		insBeforeFirstChild(elem,createAnchorEscelem());
		elem.appendChild(createAnchorClelemWithText('Detach')).onclick=function(){
			removeFromParentNode(cothis);
			fnforesc();
		};
		function listverbstoreplacewith(arrbuf){
			fnforesc();
			var u8b=new Uint8Array(arrbuf,OFF);
			var listoftu=[];
			readlistofidstrtillend(u8b,listoftu);
			var elem=document.createElement('div');
			insBeforeFirstChild(elem,createAnchorEscelem());
			for(let tu of listoftu){
				elem.appendChild(createAnchorClelemWithText(tu.s)).onclick=function(){
					var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,17]),tu.i]);
					for(var ibcle of getlowerdiv().getElementsByClassName('ibclelem'))
						if(ve!==ibcle)bloborarr=new Blob([bloborarr,defwm.get(ibcle)]);
					postreqforarraybuffer(bloborarr,function(arrbuf){
						if(arrbuf.byteLength!==OFF)
							alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
						else{
							fnforesc();
							ve.text=tu.s;
							defwm.set(ve,tu.i);
						}
					});
				};
			}
			bodyhidefirstinsertfirstfocusonfirst(elem);
		}
		elem.appendChild(createAnchorClelemWithText('Replace with a higher verb (entailed by this verb)')).onclick=function(){
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,13]),defwm.get(ve)]),listverbstoreplacewith);
		};
		elem.appendChild(createAnchorClelemWithText('Replace with a lower verb (entailing this verb)')).onclick=function(){
			postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,14]),defwm.get(ve)]),listverbstoreplacewith);
		};
		bodyhidefirstinsertfirstfocusonfirst(elem);
	};
	return ve;
}
function refreshsaverbsreadonly(sid,elem){
	for(var etbr of makeArray(elem.getElementsByClassName('ibelem')))
		removeFromParentNode(etbr);
	getsaverbsattachedtosense(sid,function(listoftu){
		for(var tu of listoftu)
			elem.appendChild(createDiv(tu.s,'ibelem'));
	});
}
function getsaverbsattachedtosense(sid,cb){
	postreqforarraybuffer(new Blob([new Uint8Array([tc.specialized_sql,20]),sid]),function(arrbuf){
		var listoftu=[];
		var off=OFF;
		for(;arrbuf.byteLength>off;){
			var id=new Uint8Array(arrbuf,off,8);
			off+=8;
			var len=new DataView(arrbuf,off,4).getUint32(0,true);
			off+=4;
			listoftu.push({i:id,s:deftextdec.decode(new DataView(arrbuf,off,len))});
			off+=len;
		}
		cb(listoftu);
	});
}
var textinseinprogress;
function oi_addverb(){
	if(this.value && !textinseinprogress){
		var cothis=this;
		var cothisv=this.value;
		var addverblabel=this.parentNode;
		textinseinprogress=true;
		postreqforarraybufferwofreezeunfreeze(new Blob([new Uint8Array([tc.specialized_sql,21]),cothisv]),function(arrbuf){
			textinseinprogress=undefined;
			if(cothis.value===cothisv){
				while(addverblabel.nextSibling&&addverblabel.nextSibling.classList.contains('ibclelem'))
					removeFromParentNode(addverblabel.nextSibling);
				var off=OFF;
				var refnode=addverblabel;
				for(;arrbuf.byteLength>off;){
					let id=new Uint8Array(arrbuf,off,8);
					off+=8;
					var len=new DataView(arrbuf,off,4).getUint32(0,true);
					off+=4;
					refnode=refnode.insertAdjacentElement('afterend',createAnchorIbclelemWithText(deftextdec.decode(new DataView(arrbuf,off,len))));
					refnode.onclick=function(){
						var coclicked=this;
						var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,17]),id]);
						var ibcle=addverblabel.previousSibling;
						while(ibcle.classList.contains('ibclelem')){
							if(!ibcle.style.borderStyle){
								var vid=defwm.get(ibcle);
								if(!strictCmpList(vid,id,8))return;
								bloborarr=new Blob([bloborarr,vid]);
							}
							ibcle=ibcle.previousSibling;
						}
						postreqforarraybuffer(bloborarr,function(arrbuf){
							if(arrbuf.byteLength!==OFF)
								alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
							else{
								coclicked.previousSibling.focus();
								addverblabel.insertAdjacentElement('beforebegin',coclicked);
								defwm.set(coclicked,id);
								coclicked.onclick=function(){this.nextSibling.focus();removeFromParentNode(this);};
							}
						});
					};
					off+=len;
				}
			}else oi_addverb.call(cothis);
		});
	}
}
function reforbmpage(bmid,fntextupdate,rowforremoval){
	postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'S'.charCodeAt(0)]),bmid]),function(arrbuf){
		var len=new DataView(arrbuf,OFF,4).getUint32(0,true);
		if(len){
			var off=OFF+4;
			var url=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
			var len=new DataView(arrbuf,off,4).getUint32(0,true);off+=4;
			var des=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
			var len=new DataView(arrbuf,off,4).getUint32(0,true);off+=4;
			var fnm=deftextdec.decode(new DataView(arrbuf,off,len));off+=len;
			var listoftu=[];
			for(;off!==arrbuf.byteLength;){
				var id=new Uint8Array(arrbuf,off,8);
				off+=8;
				var len=new DataView(arrbuf,off,4).getUint32(0,true);
				off+=4;
				listoftu.push({i:id,s:deftextdec.decode(new DataView(arrbuf,off,len))});
				off+=len;
			}
			if(fntextupdate)fntextupdate(url,des);
			showbmpage(bmid,listoftu,url,des,fnm,fntextupdate,rowforremoval);
		}else{
			alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+4)));
		}
	});
}
function showbmpage(bmid,listoftu,urlstr,descstr,fnmstr,fntextupdate,rowforremoval){
	var elem=document.createElement('div');
	insBeforeFirstChild(elem,createAnchorEscelem(''));
	if(fnmstr){
		elem.appendChild(createDiv('HTML archive: '+fnmstr+'.htm',''));
		elem.appendChild(createAnchorClelemWithText('(Delete This File)')).onclick=function(){
			var cothis=this;
			postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'D'.charCodeAt(0)]),urlstr]),alertifarrbufneoff,function(){
				removeFromParentNode(cothis.previousSibling);
				removeFromParentNode(cothis);
			});
		};
	}
	appendLabelAndInput(elem,'URL: ');
	elem.lastChild.lastChild.value=urlstr;
	var urltextin=elem.lastChild.lastChild;
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!urltextin.value)return;
		sfunc();
	};
	appendLabelAndInput(elem,'Description: ');
	elem.lastChild.lastChild.value=descstr;
	var desctextin=elem.lastChild.lastChild;
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!urltextin.value)return;
		sfunc();
	};
	function sfunc(){
		if(bmid)var cobmid=bmid;
		else var cobmid=new Uint8Array(8);
		if(urltextin.value.indexOf('\0')!==-1 || desctextin.value.indexOf('\0')!==-1){
			alertmsg('Invalid input: Unexpected null character.');
			return;
		}
		var bloborarr=new Blob([new Uint8Array([tc.bookmark,'U'.charCodeAt(0)]),cobmid,urltextin.value,new Uint8Array(1),desctextin.value,new Uint8Array(5)]);
		var ibcle=addverblabel.previousSibling;
		var listofvid=[];
		while(ibcle.classList.contains('ibclelem')){
			if(!ibcle.style.borderStyle){
				listofvid.push(defwm.get(ibcle));
			}else{
				bloborarr=new Blob([bloborarr,ibcle.text,new Uint8Array(1)]);
			}
			ibcle=ibcle.previousSibling;
		}
		listofvid.unshift(bloborarr,new Uint8Array(1));
		postreqforarraybuffer(new Blob(listofvid),function(arrbuf){
			if(arrbuf.byteLength===OFF){
				showtrmsg('Successfully saved.');
				if(fntextupdate)fntextupdate(urltextin.value,desctextin.value);
				//reopen bc you might have new verbs to create, you need to get their ids
				postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'c'.charCodeAt(0)]),blobStrWithLenLE(urltextin.value)]),function(arrbuf){
					fnforesc();
					if(arrbuf.byteLength!==OFF){
						var listoftu=[];
						var bmid=new Uint8Array(arrbuf,OFF,8);
						var off=OFF+8;
						var len=new DataView(arrbuf,off,4).getUint32(0,true);
						off+=4;
						var desindb=deftextdec.decode(new DataView(arrbuf,off,len));
						for(;;){
							off+=len;
							var id=new Uint8Array(arrbuf,off,8);
							off+=8;
							if(!isrealid(id))break;
							len=new DataView(arrbuf,off,4).getUint32(0,true);
							off+=4;
							listoftu.push({i:id,s:deftextdec.decode(new DataView(arrbuf,off,len))});
						}
						showbmpage(bmid,listoftu,urltextin.value,desindb,deftextdec.decode(new DataView(arrbuf,off,arrbuf.byteLength-off-1)),fntextupdate,rowforremoval);
					}else{
						alertmsg('Bookmark not found.');
					}
				});
			}else{
				alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
			}
		});
	}
	elem.appendChild(createAnchorClelemWithText('Save')).onclick=function(){
		if(!urltextin.value)return;
		sfunc();
	};
	for(var tu of listoftu){
		elem.appendChild(createAnchorIbclelemWithText(tu.s)).onclick=function(){this.nextSibling.focus();removeFromParentNode(this);};
		defwm.set(elem.lastChild,tu.i);
	}
	appendLabelAndInput(elem,'Add Verb: ');
	var addverblabel=elem.lastChild;
	elem.lastChild.lastChild.oninput=oi_addverb;
	elem.lastChild.lastChild.onkeydown=function(ke){
		if(ke.keyCode!==0x0d)return;
		if(!this.value)return;
		var cothis=this;
		var cothisv=this.value;
		var bloborarr=new Blob([new Uint8Array([tc.specialized_sql,22]),cothisv,new Uint8Array(1)]);
		var ibcle=addverblabel.previousSibling;
		while(ibcle.classList.contains('ibclelem')){
			if(ibcle.text===cothisv) return;
			if(!ibcle.style.borderStyle){
				var vid=defwm.get(ibcle);
				bloborarr=new Blob([bloborarr,vid]);
			}
			ibcle=ibcle.previousSibling;
		}
		postreqforarraybuffer(bloborarr,function(arrbuf){
			if(arrbuf.byteLength===OFF){
				addverblabel.insertAdjacentElement('beforebegin',createAnchorIbclelemWithText(cothisv)).onclick=function(){this.nextSibling.focus();removeFromParentNode(this);};
				addverblabel.previousSibling.style.borderStyle='dotted';
				cothis.value='';
			}else if(new Uint8Array(arrbuf,OFF,1)[0]){
				alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
			}else{
				addverblabel.insertAdjacentElement('beforebegin',createAnchorIbclelemWithText(cothisv)).onclick=function(){this.nextSibling.focus();removeFromParentNode(this);};
				defwm.set(addverblabel.previousSibling,new Uint8Array(arrbuf,OFF));
				cothis.value='';
			}
		});
	};
	if(bmid){
		elem.appendChild(createAnchorClelemWithText('Delete Bookmark')).onclick=function(){
			postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'d'.charCodeAt(0)]),urlstr]),alertifarrbufneoff,function(){
				if(rowforremoval)removeFromParentNode(rowforremoval);
				fnforesc();
			});
		};
	}
	bodyhidefirstinsertfirstfocusonfirst(elem);
	addverblabel.lastChild.focus();
}
function getlowerdiv(){
	return document.body.children[1];
}
function getcurrentdiv(){
	return document.body.firstElementChild;
}




