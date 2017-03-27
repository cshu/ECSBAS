chrome.runtime.onMessage.addListener(function(message, sender, sendResponse){
	//window.open('http://127.0.0.1:28422/');
	//chrome.tabs.executeScript({code:'window.open()'});
	//chrome.tabs.duplicate(sender.tab.id);
	if(message){
		chrome.tabs.create({url:message});
	}else{
		chrome.tabs.remove(sender.tab.id);
	}
});
