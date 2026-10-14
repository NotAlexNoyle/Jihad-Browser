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
// Modified by the Jihad Browser project (2026), per Apache-2.0 section 4(b): this file differs from
// HP's isis-browser original. The changes are this repository's git history for this file.

enyo.kind({
	name: "CertificateDetail",
	kind: enyo.VFlexBox,
	published: {
		cert: ""
	},
	components: [
		{name: "commonName"},
		{kind:"RowGroup", caption:$L("Issued By"), components:[
			{name:"displayIssuerCommonName"}
		]},
		{kind:"RowGroup", caption:$L("Start Date"), components:[
			{name:"startDate"}
		]},
		{kind:"RowGroup", caption:$L("Expiration Date"), components:[
			{name:"expirationDate"}
		]},
		{kind: "Divider", caption: $L("Subject information")},
		{kind:"RowGroup", name:"subjectAltNameGroup", caption:$L("Subject Alt Name"), components:[
			{name:"subjectAltName"}
		]},
		{kind:"RowGroup", name:"subjectCountryGroup", caption:$L("Country"), components:[
			{name:"subjectCountry"}
		]},
		{kind:"RowGroup", name:"subjectStateGroup", caption:$L("State/Province"), components:[
			{name:"subjectState"}
		]},
		{kind:"RowGroup", name:"subjectLocationGroup", caption:$L("Location"), components:[
			{name:"subjectLocation"}
		]},
		{kind:"RowGroup", name:"subjectOrganizationGroup", caption:$L("Organization"), components:[
			{name:"subjectOrganization"}
		]},
		{kind:"RowGroup", name:"subjectCommonNameGroup", caption:$L("Common Name"), components:[
			{name:"subjectCommonName"}
		]},
		{kind: "Divider", caption: $L("Issuer Information")},
		{kind:"RowGroup", name:"issuerCountryGroup", caption:$L("Country"), components:[
			{name:"issuerCountry"}
		]},
		{kind:"RowGroup", name:"issuerStateGroup", caption:$L("State/Province"), components:[
			{name:"issuerState"}
		]},
		{kind:"RowGroup", name:"issuerLocationGroup", caption:$L("Location"), components:[
			{name:"issuerLocation"}
		]},
		{kind:"RowGroup", name:"issuerOrganizationGroup", caption:$L("Organization"), components:[
			{name:"issuerOrganization"}
		]},
		{kind:"RowGroup", name:"issuerCommonNameGroup", caption:$L("Common Name"), components:[
			{name:"issuerCommonName"}
		]},
		{kind:"RowGroup", caption:$L("Serial Number"), components:[
			{name:"serialNumber"}
		]},
		{kind:"RowGroup", caption:$L("Version"), components:[
			{name:"version"}
		]},
		{kind:"RowGroup", caption:$L("Signature Algorithm"), components:[
			{name:"signatureAlgorithm"}
		]},
//		{kind:"RowGroup", caption:$L("Signature"), components:[
//			{name:"signature", kind: "RichText", richContent: false}
//		]},
		{kind:"RowGroup", caption:$L("Public Key Algorithm"), components:[
			{name:"publicKeyAlgorithm"}
		]},
		// What the daemon adds for a page that is loaded (the address bar's padlock): the
		// certificate's fingerprint, and the connection the page came over.
		{kind:"RowGroup", name:"fingerprintGroup", caption:$L("SHA-256 Fingerprint"), showing: false, components:[
			{name:"fingerprint", style: "word-wrap: break-word; font-size: 14px;"}
		]},
		{kind:"RowGroup", name:"connectionGroup", caption:$L("Connection"), showing: false, components:[
			{name:"connection"}
		]},
		{kind:"RowGroup", name:"trustGroup", caption:$L("Trust"), showing: false, components:[
			{name:"trust"}
		]}
//		{kind:"RowGroup", caption:$L("Public Key"), components:[
//			{name:"publicKey", kind: "RichText", richContent: false}
//		]}
	],
	formatCertDate: function(inText) {
		var d = new Date(inText);
		if (isNaN(d.getTime())) {
			return enyo.string.escapeHtml(String(inText));
		}
		return new enyo.g11n.DateFmt({date:'long'}).format(d);
	},
	certChanged: function() {
		var alternate_name="";
		this.cert.displayCommonName = "";
		this.cert.displayIssuerCommonName = "";
		
		if(this.cert.subject) {
			this.cert.displayCommonName = this.cert.subject.commonname ? this.cert.subject.commonname : this.cert.subject.organization;
		}
		if(this.cert.issuer) {
			this.cert.displayIssuerCommonName = this.cert.issuer.commonname ? this.cert.issuer.commonname : this.cert.issuer.organization;
		}
		
		if(this.cert.displayCommonName) {
			this.$.commonName.setContent(enyo.string.escapeHtml(this.cert.displayCommonName));
		}
		else {
			for(var j in this.cert.subject.altname){
				alternate_name += this.cert.subject.altname[j] + ' ';
			}
			// (was `this.$.certName`, a control that does not exist: a certificate with neither
			// a common name nor an organization threw here and the view never opened)
			this.$.commonName.setContent(enyo.string.escapeHtml(alternate_name));
		}
		
		// (Set whether or not present: the view is reused, and a certificate lacking a field
		// must not show the previous certificate's.)
		this.$.displayIssuerCommonName.setContent(enyo.string.escapeHtml(this.cert.displayIssuerCommonName || ""));
		
		// The dates arrive as text. Formatted when they parse; shown as given when they do not
		// (the daemon sends the engine's own local-time string, which this WebKit may not parse).
		this.$.startDate.setContent(this.cert.startdate ? this.formatCertDate(this.cert.startdate) : "");
		this.$.expirationDate.setContent(this.cert.expiredate ? this.formatCertDate(this.cert.expiredate) : "");
		
		if(this.cert.subject) {
		
			alternate_name="";
			for(var j in this.cert.subject.altname) {
				alternate_name += this.cert.subject.altname[j] + ' ';
			}
			this.$.subjectAltNameGroup.setShowing(alternate_name ? true : false);
			this.$.subjectAltName.setContent(enyo.string.escapeHtml(alternate_name));
		
			this.$.subjectCountryGroup.setShowing(this.cert.subject.country ? true : false)
			this.$.subjectCountry.setContent(enyo.string.escapeHtml(this.cert.subject.country));
		
			this.$.subjectStateGroup.setShowing(this.cert.subject.state ? true : false);
			this.$.subjectState.setContent(enyo.string.escapeHtml(this.cert.subject.state));
		
			this.$.subjectLocationGroup.setShowing(this.cert.subject.location ? true : false);
			this.$.subjectLocation.setContent(enyo.string.escapeHtml(this.cert.subject.location));
		
			this.$.subjectOrganizationGroup.setShowing(this.cert.subject.organization);
			this.$.subjectOrganization.setContent(enyo.string.escapeHtml(this.cert.subject.organization));
		
			this.$.subjectCommonNameGroup.setShowing(this.cert.subject.commonname ? true : false);
			this.$.subjectCommonName.setContent(enyo.string.escapeHtml(this.cert.subject.commonname));
		}
		
		if(this.cert.issuer) {
			this.$.issuerCountryGroup.setShowing(this.cert.issuer.country ? true : false);
			this.$.issuerCountry.setContent(enyo.string.escapeHtml(this.cert.issuer.country));
		
			this.$.issuerStateGroup.setShowing(this.cert.issuer.state ? true : false);
			this.$.issuerState.setContent(enyo.string.escapeHtml(this.cert.issuer.state));
		
			this.$.issuerLocationGroup.setShowing(this.cert.issuer.location ? true : false);
			this.$.issuerLocation.setContent(enyo.string.escapeHtml(this.cert.issuer.location));
		
			this.$.issuerOrganizationGroup.setShowing(this.cert.issuer.organization ? true : false);
			this.$.issuerOrganization.setContent(enyo.string.escapeHtml(this.cert.issuer.organization));
		
			this.$.issuerCommonNameGroup.setShowing(this.cert.issuer.commonname ? true : false);
			this.$.issuerCommonName.setContent(enyo.string.escapeHtml(this.cert.issuer.commonname));
		}
		
		this.$.serialNumber.setContent(enyo.string.escapeHtml(this.cert.serialNumber !== undefined ? String(this.cert.serialNumber) : ""));
		this.$.version.setContent(enyo.string.escapeHtml(this.cert.version !== undefined ? String(this.cert.version) : ""));
		// A view is reused: whatever the last certificate set here must not outlive it.
		this.$.signatureAlgorithm.setContent("");
		this.$.publicKeyAlgorithm.setContent("");

		this.$.fingerprintGroup.setShowing(this.cert.sha256 ? true : false);
		this.$.fingerprint.setContent(enyo.string.escapeHtml(this.cert.sha256 ? String(this.cert.sha256) : ""));
		var conn = this.cert.connection, bits = [];
		if (conn) {
			if (conn.protocol) { bits.push(String(conn.protocol)); }
			if (conn.cipher) { bits.push(String(conn.cipher)); }
			if (conn.keyLength) { bits.push(conn.keyLength + $L("-bit keys")); }
		}
		this.$.connectionGroup.setShowing(bits.length > 0);
		this.$.connection.setContent(enyo.string.escapeHtml(bits.join(", ")));
		// Whether the certificate was believed, and whether the page counts as secure.
		var warn = [];
		if (this.cert.overridden || this.cert.untrusted || this.cert.mismatch || this.cert.badTime) {
			warn.push(this.cert.mismatch ? $L("This certificate was issued for a different site.") :
				this.cert.badTime ? $L("This certificate is not valid at this time.") :
				$L("This certificate could not be verified."));
			if (this.cert.overridden) { warn.push($L("You chose to trust it.")); }
		}
		if (this.cert.broken) { warn.push($L("This page is not fully secure: part of it, or the connection itself, is not protected.")); }
		this.$.trustGroup.setShowing(warn.length > 0);
		this.$.trust.setContent(enyo.string.escapeHtml(warn.join(" ")));
		
		if(this.cert.signature) {
			if(this.cert.signature.algorithm) {
				this.$.signatureAlgorithm.setContent(enyo.string.escapeHtml(this.cert.signature.algorithm));
			}
//			this.$.signature.setValue(enyo.string.escapeHtml(this.cert.signature.hexValue));
		}
		
		if(this.cert.publicKey) {
			if(this.cert.publicKey.algorithm) {
				this.$.publicKeyAlgorithm.setContent(enyo.string.escapeHtml(this.cert.publicKey.algorithm));
			}
//			this.$.publicKey.setValue(enyo.string.escapeHtml(this.cert.publicKey.hexValue));
		}
	}
});


enyo.kind({
	name: "CertificateDialog",
	kind: "ModalDialog",
	caption: $L("View Certificate"),
	width: "450px",
	layoutKind: "VFlexLayout",
	published: {
		certFile: ""
	},
	events: {
		onCertLoad: "",
		onClose: ""
	},
	components: [
		{name: "getCertificateDetails", kind:"PalmService", service:"palm://com.palm.certificatemanager/",
			method:"getcertificatedetails",
			onResponse:"handleCertificate"
		},
		{kind: "Scroller", height: "500px", flex: 1,  horizontal: false, autoHorizontal: false, components: [
			{name: "certDetail", kind: "CertificateDetail"}
		]},
		{kind: "HFlexBox", pack: "justify", align: "center", components: [
			{kind: "Button", caption: $L("Done"), flex: 1, className: "enyo-button-dark", onclick: "doClose"}
		]}
	],
	// WHERE THE DETAILS COME FROM. The stock browser asks com.palm.certificatemanager (the
	// service component above, kept for a device where that works). On webOS CE a card gets
	// "Service does not exist" from it, so View Certificate never enabled. The daemon therefore
	// writes the details of the certificate it is warning about into THIS app's own directory —
	// certinfo/<name of the certificate file>.js, one call to window.jihadCertInfo — for as
	// long as the warning is open: a card may load a local file only from its own directory,
	// and only as a script (measured 2026-10-02; see render/goanna/BrowserPageGoanna.cpp).
	certFileChanged: function() {
//		this.warn("cert file: ", this.certFile);
		this.validateComponents();
		if(this.certFile) {
			var self = this, asked = this.certFile, done = false, tag = null;
			var finish = function(o) {
				if (done) { return; }
				done = true;
				window.jihadCertInfo = null;
				if (tag && tag.parentNode) { tag.parentNode.removeChild(tag); }
				if (o && self.certFile === asked) {
					self.handleCertificate(self, o);
				} else if (!o && self.certFile === asked) {
					self.$.getCertificateDetails.call({'certificateFilename': asked});
				}
			};
			try {
				window.jihadCertInfo = function(o) { finish(o); };
				tag = document.createElement("script");
				tag.onerror = function() { finish(null); };
				tag.src = "certinfo/" + String(asked).replace(/^.*\//, "") + ".js";
				document.getElementsByTagName("head")[0].appendChild(tag);
				window.setTimeout(function() { finish(null); }, 3000);
			} catch (e) {
				finish(null);
			}
		}
	},
	//* Show a certificate the caller already has — the loaded page's own, which the address
	//* bar's padlock asks the daemon for. (certFile, above, is the other way in: the
	//* certificate a failed load is asking about.)
	showCertificate: function(inCert) {
		this.validateComponents();
		this.$.certDetail.setCert(inCert);
	},
	handleCertificate: function(inSender, inResponse) {
//		this.warn("cert details: ", enyo.json.stringify(inResponse));
		if(inResponse.returnValue) {
			this.$.certDetail.setCert(inResponse);
			this.doCertLoad();
		}
	}
});
