browser.storage.local.get('ecsbasp').then((u)=>{
	var ue=u.ecsbasp;
	if(ue){
		var dotind=ue.indexOf('.');
		var sc=parseFloat(ue.slice(0,dotind));
		var ecsbasp=ue.slice(dotind+1);
	}else{
		var sc=0x59;//Y
		var ecsbasp='http://127.0.0.1:28422/';
	}
	function bmfn(){
		try{//debug
			//var wind=window.open(browser.extension.getURL('bm.htm'));
			//cannot use nul as separator because title can contain nul
			//var wind=
			if(document.doctype){
				window.open(ecsbasp+'bm',document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+new XMLSerializer().serializeToString(document.doctype)+document.documentElement.outerHTML);
			}else{
				window.open(ecsbasp+'bm',document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+document.documentElement.outerHTML);
			}
		}catch(e){
			console.log(e);
		}
	}
	window.addEventListener('keydown',function(ke){
		if(ke.ctrlKey&&ke.keyCode===sc){
			bmfn();
		}
	},true);
	window.addEventListener('click',function(e){
		if(e.detail>2){
			bmfn();
		}
	},true);
});
