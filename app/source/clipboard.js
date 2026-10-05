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

//* Put inText on the clipboard and say whether that worked (enyo.setClipboard above does the
//* same and does not say).
enyo.jihadSetClipboard = function(inText) {
	var n = document.createElement("textarea"), ok = false;
	n.style.cssText = "position: absolute; height: 0px; width: 0px;";
	n.value = inText;
	document.body.appendChild(n);
	try {
		n.select();
		ok = document.execCommand("cut") !== false;
	} catch (e) {
		ok = false;
	}
	document.body.removeChild(n);
	return ok;
};

//* Read the clipboard and hand its text to inCallback: a throwaway textarea and the system's
//* paste into it. enyo.dom.getClipboard does the same and reads the textarea one tick later,
//* but the paste takes anything from a few milliseconds to more than 60 (measured in the
//* Mochi shell, same platform call), so this watches the textarea for up to 600 ms before
//* calling the clipboard empty. Whatever happens, the textarea goes, the keyboard is given
//* back and the callback is called exactly once.
enyo.jihadGetClipboard = function(inCallback) {
	var n = document.createElement("textarea"), tries = 0, done = false;
	function finish(inText) {
		if (done) {
			return;
		}
		done = true;
		try { n.blur(); } catch (e) {}
		try { enyo.keyboard.resume(); } catch (e) {}
		if (n.parentNode) {
			n.parentNode.removeChild(n);
		}
		inCallback(inText);
	}
	function look() {
		if (!n.value && ++tries < 15) {
			setTimeout(look, 40);
			return;
		}
		finish(n.value);
	}
	try {
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
	} catch (e) {
		finish("");
		return;
	}
	setTimeout(look, 20);
};

//* inText as the pieces a paste is sent in. One message to the engine holds 16 KB and a
//* longer string is dropped whole (the IPC's own limit), so each piece is at most 12000
//* bytes of UTF-8, and no piece ends inside a surrogate pair.
enyo.jihadPasteChunks = function(inText) {
	var out = [], start = 0, bytes = 0, i, c, n;
	for (i = 0; i < inText.length; i++) {
		c = inText.charCodeAt(i);
		n = c < 0x80 ? 1 : (c < 0x800 ? 2 : 3);
		// A low surrogate stays with the high one before it.
		if (bytes + n > 12000 && !(c >= 0xDC00 && c <= 0xDFFF)) {
			out.push(inText.substring(start, i));
			start = i;
			bytes = 0;
		}
		bytes += n;
	}
	if (start < inText.length) {
		out.push(inText.substring(start));
	}
	return out;
};
