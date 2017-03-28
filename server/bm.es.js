
function closecurrent(){
	if(opener){close();}
	else{
		document.body.style.display='none';//document.body.innerHTML='' causes err, bc unfreeze will remove last child
		window.postMessage(0,'*');
	}
}
var bmid;
function odclh() {
	if(!name){
		setTimeout(odclh,300);
		return;
	}
	//document.referrer sometimes is the same as urlstr, but sometimes not
	var dotind=name.indexOf('.',1);
	var len=parseFloat(name.slice(0,dotind));
	var off=dotind+1+len;
	var urlstr=name.slice(dotind+1,off);

	var dotind=name.indexOf('.',off+1);
	var len=parseFloat(name.slice(off,dotind));
	var off=dotind+1+len;
	var descstr=name.slice(dotind+1,off);

	var htmlstr=name.slice(off);
	postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'c'.charCodeAt(0)]),blobStrWithLenLE(urlstr),htmlstr]),function(arrbuf){
		var elem=document.createElement('div');
		insBeforeFirstChild(document.body,elem);
		var listoftu=[];
		if(arrbuf.byteLength!==OFF){
			bmid=new Uint8Array(arrbuf,OFF,8);
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
			if(new Uint8Array(arrbuf,arrbuf.byteLength-1)[0]){
				var clnm='yellowbg';
				elem.appendChild(createDiv('Save to overwrite the record.',clnm));
			}else{
				var clnm='bluebg';
				elem.appendChild(createDiv('No difference from the saved HTML archive.',clnm));
			}
			if(arrbuf.byteLength!==off+1){
				elem.appendChild(createDiv('HTML archive: '+deftextdec.decode(new DataView(arrbuf,off,arrbuf.byteLength-off-1))+'.htm',clnm));
				elem.appendChild(createAnchorClelemWithText('(Delete This File)')).onclick=function(){
					var cothis=this;
					postreqforarraybuffer(new Blob([new Uint8Array([tc.bookmark,'D'.charCodeAt(0)]),urlstr]),alertifarrbufneoff,function(){
						removeItAndPreviousSiblings(cothis);
						insBeforeFirstChild(elem,createDiv('Save to overwrite the record.','yellowbg'));
					});
				};
			}
		}
		//elem.appendChild(createAnchorClelemWithText('Recheck')).onclick=function(){
		//	//todo
		//};
		appendLabelAndInput(elem,'URL: ');
		elem.lastChild.lastChild.value=urlstr;
		var urltextin=elem.lastChild.lastChild;
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!urltextin.value)return;
			sfunc(htmlstr);
		};
		appendLabelAndInput(elem,'Description: ');
		elem.lastChild.lastChild.value=descstr;
		var desctextin=elem.lastChild.lastChild;
		elem.lastChild.lastChild.onkeydown=function(ke){
			if(ke.keyCode!==0x0d)return;
			if(!urltextin.value)return;
			sfunc(htmlstr);
		};
		if(bmid && desindb!==descstr){
			elem.appendChild(createDiv('Description of saved record: '+desindb,'yellowbg'));
		}
		function sfunc(hstr){
			if(bmid)var cobmid=bmid;
			else var cobmid=new Uint8Array(8);
			if(urltextin.value.indexOf('\0')!==-1 || desctextin.value.indexOf('\0')!==-1){
				alertmsg('Invalid input: Unexpected null character.');
				return;
			}
			var bloborarr=new Blob([new Uint8Array([tc.bookmark,'U'.charCodeAt(0)]),cobmid,urltextin.value,new Uint8Array(1),desctextin.value,new Uint8Array(1),blobStrWithLenLE(hstr)]);
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
					closecurrent();
				}else{
					alertmsg(deftextdec.decode(new DataView(arrbuf,OFF+1)));
				}
			});
		};
		elem.appendChild(createAnchorClelemWithText('Save')).onclick=function(){
			if(!urltextin.value)return;
			sfunc('');
		};
		elem.appendChild(createAnchorClelemWithText('Save (including HTML archive)')).onclick=function(){
			if(!urltextin.value)return;
			sfunc(htmlstr);
		};
		elem.lastChild.focus();
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
					closecurrent();
				});
			};
		}
	});
}
document.addEventListener("DOMContentLoaded", odclh);
//window.addEventListener('message',function(e){
//	console.log(e.origin);
//	console.log(e.source);
//	console.log('finished');
//});
