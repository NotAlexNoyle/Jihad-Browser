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

enyo.kind({
	name: "BrowserContextMenu",
	kind: "PopupSelect",
	published: {
		view: ""
	},
	events: {
		onItemClick: ""
	},
	tapInfo: {link: true, image: true},
	linkItems: [
		{caption: $L("Open In New Card"), value:"newCardClick"},
		{caption: $L("Share Link"), value:"shareLinkClick"},
		{caption: $L("Copy URL"), value:"copyLinkClick"}
	],
	imageItems: [
		{caption: $L("Copy To Photos"), value: "copyToPhotosClick"},
		{caption: $L("Share Image"), value: "shareImageClick"},
		{caption: $L("Set Wallpaper"), value: "setWallpaperClick"}
	],
	openAtTap: function(inEvent, inTapInfo) {
		this.tapPosition = {left: inEvent.pageX, top: inEvent.pageY};
		this.tapInfo = inTapInfo;
		if (!this.view) {
			return;
		}
		var items = this.makeItems();
		if (items) {
			this.setItems(items);
			this.openNear(this.tapPosition);
		}
	},
	//* The long-press menu on anything that is not a link or an image: what a desktop browser
	//* offers on a right-click. `inState` is what the engine found under the finger —
	//* {editable, selected} — plus what this card knows: {canGoBack, canGoForward}.
	//* The values are method names on the Browser kind, as for links and images.
	openForPage: function(inPosition, inState) {
		var s = inState || {}, items = [];
		if (s.editable) {
			items.push({caption: $L("Paste"), value: "pagePasteClick"});
			if (s.selected) {
				items.push({caption: $L("Copy"), value: "pageCopyClick"});
				items.push({caption: $L("Cut"), value: "pageCutClick"});
			}
			items.push({caption: $L("Select All"), value: "pageSelectAllClick"});
		} else {
			if (s.selected) {
				items.push({caption: $L("Copy"), value: "pageCopyClick"});
			}
			items.push({caption: $L("Select All"), value: "pageSelectAllClick"});
			if (s.canGoBack) {
				items.push({caption: $L("Back"), value: "goBack"});
			}
			if (s.canGoForward) {
				items.push({caption: $L("Forward"), value: "goForward"});
			}
			items.push({caption: $L("Reload"), value: "reloadClick"});
			items.push({caption: $L("Add Bookmark"), value: "pageBookmarkClick"});
			items.push({caption: $L("Find on Page"), value: "showFind"});
		}
		this.tapPosition = inPosition;
		this.tapInfo = {};
		this.setItems(items);
		this.openNear(this.tapPosition);
	},
	makeItems: function() {
		var items;
		if (this.tapInfo.isLink) {
			var uri = enyo.uri.parseUri(this.tapInfo.linkUrl);
			if (uri.scheme && enyo.uri.isValidScheme(uri)) {
				items = [].concat(this.linkItems);
			}
		}
		if (this.tapInfo.isImage) {
			items = (items || []).concat(this.imageItems);
		}
		return items;
	},
	menuItemClick: function(inSender) {
		this.doItemClick(inSender.getValue(), this.tapInfo, this.tapPosition);
		this.close();
	}
});
