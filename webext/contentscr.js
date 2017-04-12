function set_lastargasfuncforthen(arg0,arg1){//mitigate incompatibility between firefox n chrome
	//chrome.storage.local.set(arg0).then(arg1);
	chrome.storage.local.set(arg0,arg1);
}
function get_lastargasfuncforthen(arg0,arg1){//mitigate incompatibility between firefox n chrome
	//chrome.storage.local.get(arg0).then(arg1);
	chrome.storage.local.get(arg0,arg1);
}
if(window.location.pathname==='/cef9fd58ade54ca086ac66669f49dc86'){
	if(!window.name){
		window.addEventListener('message',function(e){
			if(e.source!==window)return;
			chrome.runtime.sendMessage(0);
		});
		get_lastargasfuncforthen('ecsbash',(u)=>{
			if(!u.ecsbash)return;
			window.name=u.ecsbash;//window.name becomes string 'undefined' if undefined is assigned to it.
			chrome.storage.local.remove('ecsbash');
		});
	}
}else{
	get_lastargasfuncforthen('ecsbasp',(u)=>{
		var ue=u.ecsbasp;
		if(ue){
			var dotind=ue.indexOf('.');
			var sc=parseFloat(ue.slice(0,dotind));
			var ecsbasp=ue.slice(dotind+1);
		}else{
			var sc=0x59;//Y
			var ecsbasp='http://127.0.0.1:28422/';
		}
		ecsbasp+='cef9fd58ade54ca086ac66669f49dc86';
		function mkstrforbm(){
			if(document.doctype){
				return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+new XMLSerializer().serializeToString(document.doctype)+document.documentElement.outerHTML;
			}else{
				return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+document.documentElement.outerHTML;
			}
		}
		function bmfn(){
			try{//debug
				//var wind=window.open(chrome.extension.getURL('bm.htm'));
				//cannot use nul as separator because title can contain nul
				//var wind=
				window.open(ecsbasp,mkstrforbm());
			}catch(e){
				console.log(e);
			}
		}
		window.addEventListener('keydown',function(ke){
			if(ke.ctrlKey&&!key.altKey&&!key.shiftKey&&ke.keyCode===sc){
				set_lastargasfuncforthen({ecsbash:mkstrforbm()},()=>{chrome.runtime.sendMessage(ecsbasp);});
			}
		},true);
		window.addEventListener('click',function(e){
			if(e.detail>2){
				//fixme this fires more than once when you click thrice.
				bmfn();
			}
		},true);
	});
}
