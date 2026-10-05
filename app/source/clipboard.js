//   Copyright 2012 Hewlett-Packard Development Company, L.P.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

enyo.setClipboard = function(inText) {
	var n = document.createElement("textarea");
	n.style.cssText = "position: absolute; height: 0px; width: 0px;";
	n.value = inText;
	document.body.appendChild(n);
	n.select();
	document.execCommand("cut");
	document.body.removeChild(n);
}

//* Read the clipboard and hand its text to inCallback: a throwaway textarea and the system's
//* paste into it. enyo.dom.getClipboard does the same and reads the textarea one tick later,
//* but the paste takes anything from a few milliseconds to more than 60 (measured in the
//* Mochi shell, same platform call), so this watches the textarea for up to 600 ms before
//* calling the clipboard empty.
enyo.jihadGetClipboard = function(inCallback) {
	var n = document.createElement("textarea"), tries = 0;
	n.style.cssText = "position: absolute; height: 0px; width: 0px;";
	n.value = "";
	document.body.appendChild(n);
	enyo.keyboard.suspend();
	n.select();
	if (window.PalmSystem && PalmSystem.paste) {
		PalmSystem.paste();
	} else {
		document.execCommand("paste");
	}
	function look() {
		if (!n.value && ++tries < 15) {
			setTimeout(look, 40);
			return;
		}
		var text = n.value;
		n.blur();
		enyo.keyboard.resume();
		document.body.removeChild(n);
		inCallback(text);
	}
	setTimeout(look, 20);
};
