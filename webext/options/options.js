browser.storage.local.get('ecsbasp').then((u)=>{
	document.body.insertAdjacentHTML('afterbegin','<table><tr><td>ECSBAS URL: </td><td><input id="ecsbasp" type="text" /></td></tr><tr><td>Shortcut: &lt;CTRL&gt; + </td><td><input id="ecsbass" type="text" value="Y" /> </td></tr></table>');
	var tin=document.getElementById('ecsbasp');
	var sin=document.getElementById('ecsbass');
	var ue=u.ecsbasp;
	if(ue){
		var dotind=ue.indexOf('.');
		var sc=parseFloat(ue.slice(0,dotind));
		tin.value=ue.slice(dotind+1);
		usin();
	}else{
		var sc=0x59;//Y
		tin.value='http://127.0.0.1:28422/';
	}
	function sfn(){browser.storage.local.set({ecsbasp:sc+'.'+document.getElementById('ecsbasp').value});}
	tin.oninput=sfn;
	sin.onkeydown=function(ke){
		ke.preventDefault();
		sc=ke.keyCode;
		usin();
		sfn();
	};
	function usin(){
		if(sc>=0x41&&sc<=0x5a) {sin.value=String.fromCharCode(sc);sin.nextSibling.nodeValue=' ';}
		else{sin.value='0x'+sc.toString(16);sin.nextSibling.nodeValue=' (Key Code)';}
	}
});
