// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// This is the browser SHELL: a Mochi-composed toolbar (back / forward / reload /
// stop + address input + menu), a load ProgressBar, the JihadWebView content
// region, and basic Popup scaffolding. Full feature parity with the Enyo 1.0 app
// (bookmarks / history / downloads / find / preferences / start page /
// alert-confirm-prompt-SSL dialogs) is the next-wave parity port (T-053); this
// shell is structured so those views slot into the popups / menu below.
//
// Contract invariant (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the UI
// drives the engine ONLY through JihadWebView's callBrowserAdapter proxy and the
// palm://com.palm.browserServer/* Luna services — the identical method-name set
// and URIs the Enyo 1.0 app uses (../../app/source/Browser.js). See
// ../../docs/IPC-CONTRACT.md.
//
// Layout: FittableRows (toolbar / progress / view). The toolbar is a
// FittableColumns whose address box is `fit: true`, so the chrome reflows to the
// panel width with no hardcoded pixels beyond the shared 1024x768 both TouchPad
// models use. The toolbar is NOT wrapped in mochi.Header — Header reserves a
// title slot and pads its client, which would keep the address field from taking
// the free width. Nav glyphs are inline base64 SVG data URIs (no image assets),
// stroked white on the dark toolbar so they read against the chrome.



// Toolbar icons: crisp PNGs (this engine renders neither the mochi.IconButton
// sprites, CSS-background SVGs, nor the needed font glyphs; and border-radius:50%
// is not a circle here). A persona recolours them via CSS `filter` or swaps a URI.
enyo.JihadIcons = {
	back: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAABZklEQVR4nO3WP27CMBQG8M8kbHSBofeoxNLLdKJqL9MD9Bxdegek/jkDA8hKHGyTENvwdYHKrQStlAQJtW+zHfuXp8R+Bv6j5RBNFyDZA0AAEEKw8Rv9AovbjRM4hqUAsFgsrquqepJSXnSG7jEp5biqKkmSWut3kkOS4hjaOzRwDBNCBCnleDAYPG82m+F2uwWA1/l8XqOF/+ILBnxmlltrNyS5XC4fo2faAX/CSCZni/VOjhVFMS7LMl+tViRJY0z72B4k2c/z/CWEwLIsvbX2oQl2cFvsFiOAVAgxcs5tkySBc+5tP7f1o4xkAgB5nt+EEKi1DiEEZll2CwDT6bTfKhijSqmJ957GGO+9p1JqshtPu0DTGLXWur+DOuditLtKEX/Tuq5ZFMU9yfR7jWwdreua3ntqrQtjzOVP5akxmmXZ3Xq9nimlrnb97WcYoT0AmM1mo127uytGhIoYP0mcJLOziQ/XcMKJTkZM8wAAAABJRU5ErkJggg==",
	forward: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAABF0lEQVR4nO3WP07DMBQG8Je4IhJHYWdm4BxsbdeepyzciClnyJI/z46fHBU7fCyxqAoCocZBiH6TZUv+ObGtZ6JLfjMAcgBqMeyzdlKs67o7rfV26lslwwBkzHzvnGMAYOZ1MjTumdZ6CwDW2oP3Hsy8SYnmRERt266997DWviyBroiImHnjvYeI/F90/itzjIYQICLjYqgx5sE5J9baUURC3/c3X6FnrySE8ExEBwCvRVEopdT1uXN+SPy6uq5vh2HQIjICgNZ6X5bl1ay/9ATrImaMeZzGMwDZUpj6s1hORFRV1TJYrBbDMLQiEpJhcUKi92oxVYx9EuwIzYmImqbZWWufYl8S7BSd2vMd/e/Q5O+ZS36aN0Yfu5TQd0NKAAAAAElFTkSuQmCC",
	reload: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAC2klEQVR4nO1Wv4skRRT+XnXv7szOjLNcJAcGgskhp9ElBgaXnImBGBkJp3+FIPgXCCYKgpEgaKDpmYt4uqCca6CyIiscYq/VXV3TW9093fUZXDU2fb07NygX7QcF3VXv5/devW7gEpcYAUn1uBwJSfmvRqItnCkAMMa8r7V+Puz//9kGZ1Fw9gFJGmNeD2fxNkaUtfa1/t6YXPdsjPmQJJumabTWX4TzzQx1Qlrrt0kyy7KPjo6Odof0dEGdnp4+kef5JySZpmldFAWzLPsjSZLFeYH2jSgAyPP8mrW2Xq/XzLLsB5LTYVOQVCTFWnvdOfd1URTUWvs0TX2g9eVhlmMFVQDgvX9jPp/v1HVtAbwqIg6AiAg7QRHxALBYLH6cTqcvNE3z02w2ExGx3vvv27a9ubFrQxZxmqbfkvRa6y/7mZ+jswMAWZa9RZJ5nv+dJMnVoiiuDmv4UE1EhMaYBYCnAUgURXdDlBe1tw8y39V1jclkciWO42uz2ez+KH0jkLBAsrrA0b8KIvTe17330SGghkokZblcnonIXwDgvX8u1I1D5b4qSVFKPRvHMaqqKpVS98f0RptGREoAdwEwiqIXtdbLB8k+XMeQhYQMX1FKsWma3xeLxa+hRH6Tw87Qx+v1Wvb3959USr0bFHl4eLhDMgorBhCJyDpN09vT6fQmHpTiUxFZA3iksdi/+J+R5NnZGfM8f6/rxiFWq9WbzrmibVsaY34juezu6FB29I50NBljDqIoujOfz2/UdY2yLO+R/BzANyQbANd3d3dfAnBrMpmgqipd1/Wtg4ODQ5JqSOemLBUAnJycXCmK4g57qKqKZVn2t+ic+zlJkht9hrZGv0lWq9Xtoii+stZmVVWxqipaa51z7p5z7p3j4+Plozjb+LHs6tCNNJJPlWX5TBzHaNv2z729vV9EpO0C3IrGDY6j8+biRWdDbP07EGju9AiA/YF+iceOfwApd0CmodeDSgAAAABJRU5ErkJggg==",
	stop: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAB/klEQVR4nO2VsY7TQBCGx75Ag2ig4ClokGhoKHgNkCguEVS8CdTQIPEMPEaEBLpHiBJFjj3r3bXX3l3zX3FeMEvCxVYKivzV2v6934x3Zkx01lkjldxmAJASUZIkSXeED3RjxqRo+k3+Wv/L11/fmsjBTYqieC6EWPT3Znt8MyKi7Xb7zBjzNcuy+6OhAFIACTO/qOuaAYCZL2NoWGdZ9tQYkwGAlPIHgAcAkqOhAC6IiIQQCwBQSrXOOTDzPIAiWKG17rqug5Tyy2azuReCHpUlEVGe55fOOSil7BAawwCgLMtPg/cnneOMiIiZ5845aK2t9x5lWb4siuKxMYZjGICLSbB9UO89qqpCURSdEEJVVYX+k58GFkPLsnxljNFSyk4IAWOM11q/HwM72Fv75L3/BqBNkuQnEdk0Tcla+z3sNbnhh4qqUVRVhbqu0TQNpJSd9x55ni+IiJbL5Z1Twn5VoxDiAzO/7qvXxS0zFZYSEa3X6z9goUCIbvrUWguttbXWDqHjCmc4aYwxudbax6V/dXV1l+h3yyilXNu2EEK87QfD8fURT5o+s4/hWcggnBkzz9u2hXMOUkqhlHo0arSFLImIdrvdO6XU52HmkW9GRJTn+ZumaVbM/GT4/ihFv6eDEQffarV6GLyjYcPNjok2QCZlNlUnGWtn/fe6BjOqYn5X7L9uAAAAAElFTkSuQmCC",
	share: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAACiUlEQVR4nN2WvW7UQBDHZ9b2+YSOi51nQKKiTJOSjxfgBahpKFEaSsQ7pEJAw0tAGRFFgoLqpERKpEhEjrxjZ5Wzz/b+KbIXWSLnu3OChJjGXzPz2/nv7qyJ/nfjPkEAPBfbMDPudkh/wrjreZn564KYGXmePw/DcPPo6OgzM18AUMxs1wEvhTkZKcuyXTgzxnw7OTnZdD7qLmGqDRORmda6tNbCGLN/fHwc3wm0DRORXQDQWs9EBFmWIU1T2zQNjDH7aZpuzGNuA1QAPBF57yqzxhhora3WuhERMcagrmtkWfZTRDbdIBdCV5EAAH64+19VVb0OggDD4ZCttS/quv7ieR4BOJhOpyX13Gpz0vViEZGXSZJspWn6qKoqVFUFEdk6Pz8f53n+ph3TG3iTicjTsixRliW01k/Wje+UFMAQQJym6QaA0M1p03KxToVBkiT3AcRnZ2ejtYEAfFfNKyI6ZOaD09PTkdvc15L5vk/MDGaeBUHwgYgOwzD86HLcmLuz0yil7hFRzMzEzHMQWlW2++gGEcXuutA6gS4xAFTzd03TBGEYekRERVEELd/a+da9gXQlHxMRAwARked5SVmWX4mIfN9PALCTlYmIW0r0Al7baDSyRERxHH8nosfz926uVj6iVu59SikfgA9gMJlMwslkEgIIiMh3i2yl/bdqhRiPx0mXg9Z6dmugtZaJyDLzUGv9jpmn1lpWSrUlZLqS9IG1dumZ2AlkZkVXso+jKNrp8r28vCSlFAEY9gFaAJzn+aeiKJ4NBoOHIlLTgnliZvi+r8qyvGDmt51FdH2c+wCIRASLljwARFHEe3t7xfb29nSFnDdbn86/7NRfmnBd6F//bfzn7TdXVcWzva+xYwAAAABJRU5ErkJggg==",
	newtab: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAADCUlEQVR4nO1VsY7kRBB9z257Znc8zLDHIRFBBiHB3okQhAj5AhKEiC8guABICfkJwk0RIKILCViJiA0uOV2EtLtMe8f23NjjrkeA57S3O+uZWwjvSS1L5ap6Xa+7q4BX+J/BbQ6Stvq8kJDUrXcjKb5NTN8mXV8wySBpD8Awz3OR3JhIkqbTKbz3InnRm/OGBATAoii+TpLky6Zp9sysV1qScs6J5M8hhIfj8fjvtb0v7rmMeZ5/I0lt22pXmJkkaTab/SopkhT1VrjW/vz8PHPOPc6y7G5Zln+Q/FFSRNI617b7RgAikpEkA/B+mqafRlHkQgj3R6PR75JikuGm6thVd+C9P5Ok+Xz+sFeSSzg7O7vXNI3qujbv/ceXFVvjWslrbgDWBexLct03mc/nd/M8fyKp6GRPJGXHx8dJkiQTSWvlNlZ1E+E6CJKMZAvgGclVXddLM5sAyADEJFcAnh0eHq4khavxL0P4AoqiuJPn+evOuTsk2+6CDL33UwBvdNJtffS977BDCwBmduyc21+tVgRwUJYlST4A8IWZxfP5/BOSZRRFMLMbk+1CCADY399/O0kSAEBRFAghIMuykXNuBABRFO2ZWb0tzy6EEQAsFotvnXNxCGEo6cFwOBxVVfUIwKPxeJw0TfM0TdN3ukvz3wmn0+l3a4P3/vPhcLjXNM1Pk8nk+0v297YRbr003aOOJWWSBqenp28BOAAQSXpT0kDSa5Icdpg+fRUKAMxs0XWKsrP/NZvN7gNIST4hWQOoASDP8zlJmJnatt1IvpFQ/+riQgiB5Afe+49IxpKCc04hhOri4sJIvuu9H6z/SfqwbVtzzkWDwWBzO9tAFkui9/4HSSqKQsvlUsvlUnVdq65rrVYrtW2rro09X1VVSZLyPH8saSSJV2fjpjM0AIjj+KvFYvFLkiS4OgbNDCEErC+IJEiCcw51Xf8J4DOSFQBeHU9bD7mqqntpmmZlWfb6OecQx3E4OTn5rWtz18h6sUmOl4jduWVew9HRUdyd667rVpt8hVvjH9tHTdc+UNHdAAAAAElFTkSuQmCC",
	bookmarks: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAD9klEQVR4nN2VT2gjdRTH3/vNJI3NJKZIi7KHkoOwuD2IK+LBlQraw4IHkV4UFC8eRBCPnnQ9qujV9eZJaPUiqOAWuwcF0d5qFE8BPSxJmpk3md/8SebP10N/U9JsGrrS0z4IGea93/fz3vv9fm+I7nfjixICoIhIzXHlzIyL4tyznbtCAEzHFZys2d3dxfb2NjFz7nneZqvV2vQ8r2BmBQCNRoO11l+urKx0AShmLs4F2t/ftxf4LQCW67ofAUBRFJg2rfVWGUdEdKaQCSqzygA4WusnATxVrVbtPM/9SqXya6fTOdzY2JiISEBEme/7mdEt6vW6AjCZ1lyUuWLmYjAYNOr1+rtxHL/pOM6l2bh2u/3H0dHRB0SkANhERMxsAyiY+dQWnAksYf1+/wnHcb6o1WpX4zgmEfkFwC2l1JiIHrIs67rjOBvLy8tfi8hAa01EZAHIZ0EL2wiAe73ew6PR6A4A+L6/Px6Pr8yJtbTWW1EU/ZvnOVzXLcIwBACISDaZTIogCJ4rY88EEhF5nvedgX1VtgoAi8h7eZ4HruteK9f0+/1HtNaHSZLkItL1ff9DrTUAYDQaPTsNVDMwi5kLz/M2Hce5HkXRnyLyFjNnAGrmAleVUg4d75kCUF9bW7uTpum3S0tLipm52WzeyLLshSRJIgDONGPeZCBmftW2bRqPxzfX19c9ABUiyoy7OM4NqTnBEQC2bft2GIYHzWZzXUReb7Vae1rrFwEMynrmAQsASwA2kySZKKW+Nxc+n4qZEBEsy3IANIjoQSKqNhqNW+Px+GOTzDUA1urq6k+tVut3U0RxCgiAmRki8gARXUqSZJymac+0sfwRgMdNO38QkWGWZUci8gwAValUekTESql1Zs4B2CbhE7vrWgCAUqqYDZzyH2ZZ9jQzd81BYGb2mbkYjUblkC5HWLFwcANgALaIHKRpmoZheNUcjFNHutvt1uas4yAI3gZQiMhn5v1dBc3uocXMGRHt2bZtJ0nyMjMXnU7nBAiA2+12YhIpk7GZGWmavmIq/rEMP7M6I6aIiEaj0eUoiiZJkgyGw+GG8VXLNk/9KwBVIiLXdV9L07QQkb8B1MqqFwKNiGUEPgGAMAwPfN9/dDqpchqV70RkK45jL01TeJ730rTOeYBsBG3P8342n5ie1vqNMAxPDe8kSS5rrd+P4xgAMBwOP70n2BxoMwzDm+W3TUQiz/Nui8ie67q/lXMziqKx7/vvlLBztXIetHwOguD5OI6/CYLAm/m4/hMEwedBEFyZXXOWLQwoBcq7JCIrSqnHarWaNZlMdL1e/4uZ47IyZs4X6Z3bTJvmzt1FvosAswFYOzs7/2+v7nv7D6hZPMoW7YwgAAAAAElFTkSuQmCC"
};

// Dark reload/stop for INSIDE the light URL bar (the white nav icons would be
// invisible there).
enyo.JihadInlineIcons = { reload: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAADb0lEQVR4nO1WTYhbVRT+zrnvJek0tVJkcKGogzD0+YcENwqNlDJTM52ZTONDKqIydaF7t+XluRUUXahk4UYo6IP+RKGzmEWyGkMNguIsWkQXbSlTUDStk8y79x4XSYbJmyS0gl3NB29xzzvvfuc759xzH7CHPQxBEAR8v7io9/x3+L6v7pasr6ywUPpsZm7puZ79f1FL/cAK8yc/919/U+bmS28BQD6fd3Y6jmOnIAi4UCyd2mkb5gdAoigyhcWTFTflvtvebBshWwSAyclJSTrvgu/7Kooi88r80pls9sAHd27f/nJ/Rr3neZ4Ow9AmgqJGo5FVqYkvnFTq1FanEzOzKyLXHGivWq22+kGNUshRFJlCsXiYmc50Om0IIQdAlctlSQRJYRgKpdOPsVKPa60BwLHWSjqTeWSLnJd7ArZ5dhHm8/muzdJp1027xpiWsC1FUbRZLpe3I+3BAsClixd//vZ89KKI/cVxHCJCy2rzIwmOJrO4i7BWq5luoekIEYm1dm3lwoVfgyDgRDr7kFwu5wIAgc46jgOA9J2/N0+40B/6vs9RFJlRhEREks1mDwjkid66AYBqtdrIBpuamrIAyFp7WWsNpdShzH73cLVavZH0HbpJHMdEoH4qOqOIkkpFZGvHYugQSBIKAOp0Ov8IsNE1yLMAJNneO7GxsUEAiBU9xaxgrWkb6BvoHpeB73Yp9H2f6/V6G5CGQASgI8eO+Qc9z5MRM5Kmp6cJgIjFEjGJFfn91vXrV3sKB+ruDNmgr/Uro/XbjnIepn36ozAMTwOgXC7n9mrWV4ZKpRIfXyguM6ujIkIQfN1sNuN8Pu/U63U9EN0wru2Dv7D0TTqd8XUcQ8R+evPaQ+83m5U46X9i8dV3hPCJYjWhdfzbVlo9/5LntcIwFAweo5GTnYIgoLW19QfdfXbFcd0XrLEwRv8E4JwhfE/GaBA/w8THmWmWmWGt/GFsZ/ZStfrDqGM07iphAHZmZuZQauKBs6zULFO3hMZoCADFCsQEsQJj9RUdmzdWvjt/uZ+hoUrGEAIIGOhGObdQWgbRMmCfZlYHAcBa2ybiq0I412n9+fHq6upf48jugnDARwBgcfG1R2OJn3QcB2zppuvaK32CMdPo3tG770Y22ah3Sdz770AQsL++TgDgeZ4M68Q93Ff8C4vThcTSxK4yAAAAAElFTkSuQmCC", stop: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAACXklEQVR4nO2VsW4TQRCG/53ZQ0LCgJJHyBtgWfJd5nQOPIPLFEiJhWiQeAHnKlpEgxIXUJOWJuVhUxoRpFyZio6CDiXO7Q4Fd9LhOMnFoUHKL22zmp3v/rmZXeBWt/rXGg6H1O/3uUEoATDlWlpUBzeJK3V9aAUQkUREBgCQJImdj6v2wjCMkiT5GEVR69rQEmZE5Ekcxz97vZ7Gcbw1D63BOnEc/9jY2FAR+dbtdldUdWF5z30xAOR5bgB4AGvM/LAoihkzj0TEZFk2qkBZlhVhGHaCIDjw3j/w3sMY87XVap3u7OwYADqf+0Lbw+GQ0jT1URRtBUEw8t6fEVHgnBtMJpNR5ayCWWvJOTcaj8eDWu7mwKpkWZYVIrLNzHs16CYRHarqJ1W9X4f1+33e39/3i2BXAueh1to95xy89x7ALyK6Z61FURSNYI2Ac9BNY8xbVb0LgIjIqeqb8Xj8sgkMOD8/l0pVvxhjTvGnoWYA4L0/BIDj42O6CtYIWLkLw7DDzJ9VdcUYY4nojvfeWGvfi8hgOp2etdvt4EbAOiwIggNVbVlroaqvVfUpM5NzrmDmXRHZnk6nZ4suh7quHIvLWl9EBsy8u2BkFo7EhcASpiLymIg+lDCut/7R0RHneT6rRsY5VxCR9d4/Z+ZRr9fzaZr6RiUtbxoFsEZEKyVsr976eZ7P2u12MJlMRs65ARFZYwwAvDo5OVlN01Qvq+BClwCwvr7+QkTe1fb+SlL9syiKniVJ8r3b7T6qn7+u6ocufOuq5J1OZ7UWu7QIzea1gizlbFndyNmt/g/9Bn89SYCL2k6MAAAAAElFTkSuQmCC" };

enyo.kind({
	name: "JihadBrowser",
	kind: "FittableRows",
	classes: "jihad enyo-fit",
	published: {
		//* Current shell URL (mirrors what the address field shows).
		url: ""
	},
	components: [
		// Toolbar built from Mochi controls, laid out with FittableColumns so the
		// address box (fit: true) absorbs the free width; the buttons keep their
		// intrinsic size, so the bar reflows on both TouchPad models. Back/forward
		// start disabled — enabled from page history state (onPageInfoChanged).
		{kind: "FittableColumns", name: "actionBar", classes: "jihad-actionbar", components: [
			{name: "back",    classes: "jihad-nav-btn disabled", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.back + "'>", ontap: "goBack"},
			{name: "forward", classes: "jihad-nav-btn disabled", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.forward + "'>", ontap: "goForward"},
			// Address field with the reload/stop control INSIDE it (Enyo-parity).
			{kind: "mochi.InputDecorator", name: "addressBox", classes: "jihad-address-box", fit: true, components: [
				{kind: "mochi.Input", name: "address", classes: "jihad-address", placeholder: "Search or type a URL", onchange: "addressEntered", onkeydown: "addressKeydown", attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false", type: "url"}},
				{name: "reloadStop", classes: "jihad-inline-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadInlineIcons.reload + "'>", ontap: "reloadOrStop"}
			]},
			{name: "share",     classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.share + "'>", ontap: "doShare"},
			{name: "newtab",    classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.newtab + "'>", ontap: "doNewCard"},
			{name: "bookmarks", classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.bookmarks + "'>", ontap: "doBookmarks"}
		]},
		// Load progress; shown only during navigation.
		{kind: "mochi.ProgressBar", name: "progress", classes: "jihad-progress", progress: 0, showing: false},
		// Rendered web content: the NPAPI-bound Enyo 2 WebView (JihadWebView.js).
		{kind: "JihadWebView", name: "view", fit: true, classes: "jihad-view",
			onLoadStarted:     "loadStarted",
			onLoadProgress:    "loadProgress",
			onLoadStopped:     "loadStopped",
			onPageInfoChanged: "pageInfoChanged",
			// Engine-driven JS dialogs (T-053): presented by JihadDialogs.
			onDialogAlert:        "showAlertDialog",
			onDialogConfirm:      "showConfirmDialog",
			onDialogPrompt:       "showPromptDialog",
			onDialogSSLConfirm:   "showSSLDialog",
			onDialogUserPassword: "showLoginDialog"
		},
		// Find-in-page bar (overlay below the toolbar); forwards to findInPage.
		{kind: "JihadFindBar", name: "findBar", onFind: "findRequested", onClose: "findClosed"},
		// Start page rendered as APP CHROME (not a size-limited WebView data: URL),
		// so the logo is the crisp bundled asset — matches the Enyo variant. Shown
		// over the (idle) WebView until the first navigation; hidden on loadStarted.
		{name: "startPage", classes: "jihad-startpage", components: [
			{tag: "img", name: "spLogo", classes: "jihad-sp-logo", attributes: {src: "icon-256x256.png"}},
			{content: "Jihad Browser", classes: "jihad-sp-title", allowHtml: false},
			{content: "Mochi UI \u2605 Enyo 2", classes: "jihad-sp-sub", allowHtml: false}
		]},
		// Overflow menu (opened from the toolbar menu button). Items launch the
		// T-053 parity views below.
		{kind: "mochi.Popup", name: "menuPopup", classes: "jihad-menu-popup", floating: true, components: [
			{classes: "jihad-menu-item", ontap: "menuNewCard",      content: "New Card"},
			{classes: "jihad-menu-item", ontap: "menuBookmarks",    content: "Bookmarks"},
			{classes: "jihad-menu-item", ontap: "menuHistory",      content: "History"},
			{classes: "jihad-menu-item", ontap: "menuDownloads",    content: "Downloads"},
			{classes: "jihad-menu-item", ontap: "menuFind",         content: "Find in Page"},
			{classes: "jihad-menu-item", ontap: "menuAddBookmark",  content: "Add Bookmark"},
			{classes: "jihad-menu-item", ontap: "menuPreferences",  content: "Preferences"}
		]},
		// Parity views (full-card overlays, hidden until opened).
		{kind: "JihadBookmarkList", name: "bookmarkList",
			onSelectItem: "navigateTo", onAddBookmark: "addCurrentBookmark", onClose: "panelClosed"},
		{kind: "JihadHistoryList", name: "historyList",
			onSelectItem: "navigateTo", onClearHistory: "clearHistory", onClose: "panelClosed"},
		{kind: "JihadDownloadList", name: "downloadList",
			onOpenItem: "openDownload", onCancelItem: "cancelDownload", onClearAll: "clearDownloads", onClose: "panelClosed"},
		{kind: "JihadPreferences", name: "preferences",
			onClearBookmarks: "clearBookmarks", onClearHistory: "clearHistory",
			onClearCookies: "clearCookies", onClearCache: "clearCache", onClose: "panelClosed"},
		// Engine-driven dialog set (alert / confirm / prompt / auth / SSL).
		{kind: "JihadDialogs", name: "dialogs", onDialogAnswer: "answerDialog"},
		// Generic info dialog (page/engine errors, Share placeholder).
		{kind: "mochi.Popup", name: "dialog", classes: "jihad-dialog", floating: true, modal: true, centered: true, components: [
			{name: "dialogTitle",   classes: "jihad-dialog-title"},
			{name: "dialogMessage", classes: "jihad-dialog-message"},
			{classes: "jihad-dialog-buttons", components: [
				{kind: "mochi.Button", decoratorLeft: "", decoratorRight: "", content: "OK", ontap: "closeDialog"}
			]}
		]}
	],

	// --- init + launch parameters -------------------------------------------
	create: function() {
		this.inherited(arguments);
		//* Session download records (download-manager status + history), rendered
		//* by the DownloadList view.
		this.downloads = [];
		//* Latest page title (from the engine), used when saving a bookmark/history.
		this._title = "";
		// webOS relaunch (a second launch of the running app, e.g. an external
		// link tap) redelivers launch params. Register best-effort listeners for
		// the platform relaunch events; harmless where they never fire.
		// [device-gated: relaunch delivery is verified on hardware.]
		if (window.PalmSystem) {
			this._relaunchBind = enyo.bind(this, "relaunchHandler");
			if (document.addEventListener) {
				document.addEventListener("webOSRelaunch", this._relaunchBind, false);
			}
			if (window.addEventListener) {
				window.addEventListener("mojo-relaunch", this._relaunchBind, false);
			}
		}
		this.applyLaunchParams(this.readLaunchParams());
		// Start with the address bar focused so the keyboard is up (all variants).
		enyo.asyncMethod(this, function() { if (this.$.address && this.$.address.focus) { this.$.address.focus(); } });
	},
	//* Read the initial launch parameters. Enyo 2 does NOT populate the Enyo-1
	//* `enyo.windowParams`; the webOS way is to parse PalmSystem.launchParams
	//* (a JSON string). Fall back to enyo.windowParams only if some bootplate
	//* set it.
	readLaunchParams: function() {
		var p = null;
		if (window.PalmSystem && window.PalmSystem.launchParams) {
			try {
				p = enyo.json.parse(window.PalmSystem.launchParams);
			} catch (e) {
				p = null;
			}
		}
		if (!p && window.enyo && enyo.windowParams) {
			p = enyo.windowParams;
		}
		return p || {};
	},
	//* Apply launch params, matching app/source/BrowserApp.js: `target` (external
	//* link / new-card launches) takes precedence over `url`; `webviewId` binds
	//* an engine-created card to this view.
	applyLaunchParams: function(p) {
		p = p || {};
		if (p.webviewId) {
			this.$.view.setIdentifier(p.webviewId);
		}
		var url = p.target || p.url;
		// Launched from the icon with no URL: show the Mochi start page so the card
		// presents content instead of a blank/forever-loading view (device: "the
		// mochi app just loads forever and doesn't open"). Only on the FIRST apply
		// (a later relaunch with no URL must not wipe the current page).
		if (url) { this.setUrl(url); }   // else the app-chrome start page overlay stays visible
	},
	relaunchHandler: function() {
		this.applyLaunchParams(this.readLaunchParams());
		return true;
	},
	urlChanged: function() {
		if (this.url) {
			this.$.view.setUrl(this.url);
			this.$.address.setValue(this.url);
		}
	},

	// --- navigation: forwarded verbatim to the frozen adapter contract -------
	// These four are the exact callBrowserAdapter method names the Enyo 1.0 app
	// uses (Browser.js). Do not add or rename (cavekit-ipc-contract R1).
	goBack:     function() { if (!this.$.back.hasClass("disabled")) this.$.view.callBrowserAdapter("goBack"); },
	goForward:  function() { if (!this.$.forward.hasClass("disabled")) this.$.view.callBrowserAdapter("goForward"); },
	reloadPage: function() { this.$.view.callBrowserAdapter("reloadPage"); },
	stopLoad:   function() { this.$.view.callBrowserAdapter("stopLoad"); },
	//* Find-in-page: the frozen adapter method. The FindBar UI that calls this
	//* is the T-053 parity port; the method is present now so the app's
	//* callBrowserAdapter set matches the Enyo 1.0 app exactly.
	find:       function(str) { this.$.view.callBrowserAdapter("findInPage", [str]); },
	//* Luna services the Enyo 1.0 app invokes from Preferences (Browser.js). The
	//* Preferences UI is the T-053 port; these are present now so the
	//* palm://com.palm.browserServer/* URI set matches the Enyo 1.0 app exactly.
	clearCookies: function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCookies', '{}'); },
	clearCache:   function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCache', '{}'); },

	// --- address bar --------------------------------------------------------
	// mochi.Input onchange fires on blur/commit; Enter does not blur on its own,
	// so also submit on the Enter keydown (keyCode 13). Both funnel through
	// submitAddress, which de-dupes so an Enter that also blurs won't double-load.
	addressEntered: function(inSender) {
		this.submitAddress();
	},
	addressKeydown: function(inSender, inEvent) {
		var code = inEvent && (inEvent.keyCode || inEvent.which);
		if (code === 13) {
			this.submitAddress();
			if (inSender.hasNode()) {
				inSender.node.blur();
			}
			return true;
		}
	},
	submitAddress: function() {
		var text = this.$.address.getValue();
		if (!text || text === this._lastSubmit) {
			return;
		}
		this._lastSubmit = text;
		this.openUrl(text);
	},
	openUrl: function(text) {
		var url = this.normalizeUrl(text);
		if (!url) {
			return;
		}
		this.url = url;
		this.$.view.setUrl(url);
	},
	//* Turn an address-bar entry into a URL, mirroring app/source/URLSearch.js
	//* (go / looksLikeHost): pass known navigational schemes through untouched
	//* (so about:blank and mailto: are not corrupted), treat a host-looking token
	//* (localhost, host:port, IPv4, host.tld[/path]) as an http:// navigation,
	//* and hand everything else to the default search.
	normalizeUrl: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v) {
			return v;
		}
		var m = v.match(/^([a-z][a-z0-9+.\-]*):/i);
		if (m) {
			if (this.isNavigableScheme(m[1].toLowerCase())) {
				return v;
			}
			// Unknown / non-navigational scheme (e.g. javascript:) -> search.
			return this.searchUrl(v);
		}
		if (this.looksLikeHost(v)) {
			return "http://" + v;
		}
		return this.searchUrl(v);
	},
	//* Schemes the address bar loads as-is. javascript:/vbscript: are excluded on
	//* purpose (typing them is treated as a search, not an in-page eval).
	isNavigableScheme: function(scheme) {
		switch (scheme) {
			case "http": case "https": case "ftp": case "file":
			case "about": case "mailto": case "tel": case "data":
			case "view-source": case "ws": case "wss": case "rtsp":
				return true;
			default:
				return false;
		}
	},
	//* Heuristic from URLSearch.looksLikeHost: does the raw input denote a host
	//* (=> navigate) rather than search terms?
	looksLikeHost: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v || /\s/.test(v)) {
			return false;
		}
		if (/^localhost(:\d+)?([\/?#].*)?$/i.test(v)) {
			return true;
		}
		if (/^\d{1,3}(\.\d{1,3}){3}(:\d+)?([\/?#].*)?$/.test(v)) {
			return true;
		}
		return /^[a-z0-9]([a-z0-9\-]*[a-z0-9])?(\.[a-z0-9]([a-z0-9\-]*[a-z0-9])?)*\.[a-z]{2,}(:\d+)?([\/?#].*)?$/i.test(v);
	},
	searchUrl: function(text) {
		return "https://duckduckgo.com/?q=" + encodeURIComponent(text);
	},

	// --- overflow menu (scaffold) -------------------------------------------
	openMenu: function(inSender) {
		// T-053 populates the menu; for now just present the (empty) popup.
		this.$.menuPopup.show();
	},

	// --- WebView (JihadWebView) callbacks -> chrome state -------------------
	hideStartPage: function() { if (this.$.startPage && this.$.startPage.hide) this.$.startPage.hide(); },
	loadStarted: function() {
		this.hideStartPage();
		this._lastProgress = 0;
		this.$.progress.setProgress(0);
		this.$.progress.setShowing(true);
		this.setStopVisible(true);
	},
	loadProgress: function(inSender, inEvent) {
		var p = (inEvent && typeof inEvent.progress === "number") ? inEvent.progress : 0;
		if (p >= this._lastProgress) {
			this.$.progress.setProgress(p);
			this._lastProgress = p;
		}
	},
	loadStopped: function() {
		this.$.progress.setShowing(false);
		this.$.progress.setProgress(0);
		this.setStopVisible(false);
		// Record the visit in the Jihad history kind (BrowserApp.pageLoadStopped
		// parity): drop any prior row for this URL, then insert the fresh one.
		this.updateHistory(this._title, this.url);
	},
	//* Page info from the engine: {title, url, canGoBack, canGoForward}. Reflect
	//* the URL in the address bar and the history state on the nav buttons. A new
	//* address entry clears the de-dupe guard so the same URL can be retyped.
	pageInfoChanged: function(inSender, inEvent) {
		if (!inEvent) {
			return;
		}
		if (inEvent.url) {
			this.url = inEvent.url;
			this.$.address.setValue(inEvent.url);
			this._lastSubmit = null;
		}
		if (typeof inEvent.title === "string" && inEvent.title) {
			this._title = inEvent.title;
		}
		if (typeof inEvent.canGoBack === "boolean") {
			this.$.back.addRemoveClass("disabled", !inEvent.canGoBack);
		}
		if (typeof inEvent.canGoForward === "boolean") {
			this.$.forward.addRemoveClass("disabled", !inEvent.canGoForward);
		}
	},
	//* Swap the reload/stop affordance during a load.
	setStopVisible: function(loading) {
		this._loading = loading;
		this.$.reloadStop.setContent("<img class=jihad-nav-img src='" + (loading ? enyo.JihadInlineIcons.stop : enyo.JihadInlineIcons.reload) + "'>");
	},
	reloadOrStop: function() {
		if (this._loading) { this.$.view.callBrowserAdapter("stopLoad"); }
		else { this.$.view.callBrowserAdapter("reloadPage"); }
	},
	//* New card (Enyo-parity "new tab").
	doNewCard: function() { if (window.enyo && enyo.windows) { enyo.windows.openWindow("index.html"); } },
	doShare: function() { this.openDialog("Share", "Copy the address from the bar to share this page."); },
	//* Toolbar menu button opens the overflow menu.
	doBookmarks: function() { this.openMenu(); },

	// --- overflow menu actions ----------------------------------------------
	menuNewCard:     function() { this.$.menuPopup.hide(); this.doNewCard(); },
	menuBookmarks:   function() { this.$.menuPopup.hide(); this.hidePanels(); this.$.bookmarkList.open(); },
	menuHistory:     function() { this.$.menuPopup.hide(); this.hidePanels(); this.$.historyList.open(); },
	menuDownloads:   function() { this.$.menuPopup.hide(); this.hidePanels(); this.$.downloadList.setDownloads(this.downloads); this.$.downloadList.open(); this.refreshDownloads(); },
	menuFind:        function() { this.$.menuPopup.hide(); this.$.findBar.show(); },
	menuAddBookmark: function() { this.$.menuPopup.hide(); this.addCurrentBookmark(); },
	menuPreferences: function() { this.$.menuPopup.hide(); this.hidePanels(); this.$.preferences.open(); },

	//* Hide every full-card overlay view.
	hidePanels: function() {
		this.$.bookmarkList.hide();
		this.$.historyList.hide();
		this.$.downloadList.hide();
		this.$.preferences.hide();
		this.$.findBar.hide();
	},
	//* A list view selected an item (bookmark/history) — navigate to it.
	navigateTo: function(inSender, inEvent) {
		var url = inEvent && inEvent.url;
		if (!url) { return; }
		this.hidePanels();
		this._lastSubmit = null;
		this.openUrl(url);
	},
	panelClosed: function() { /* the view hid itself; nothing else to do. */ },

	// --- find-in-page -------------------------------------------------------
	//* FindBar query -> the frozen findInPage adapter method (via this.find).
	findRequested: function(inSender, inEvent) {
		if (inEvent && inEvent.value) { this.find(inEvent.value); }
	},
	findClosed: function() { /* bar hid itself. */ },

	// --- engine-driven dialogs (present via JihadDialogs) -------------------
	showAlertDialog:   function(inSender, inEvent) { this.$.dialogs.showAlert(inEvent.message); },
	showConfirmDialog: function(inSender, inEvent) { this.$.dialogs.showConfirm(inEvent.message); },
	showPromptDialog:  function(inSender, inEvent) { this.$.dialogs.showPrompt(inEvent.message, inEvent.defaultValue); },
	showSSLDialog:     function(inSender, inEvent) { this.$.dialogs.showSSL(inEvent.host, inEvent.code, inEvent.certFile); },
	showLoginDialog:   function(inSender, inEvent) { this.$.dialogs.showLogin(inEvent.message); },
	//* The user answered a dialog -> hand the string args to the WebView, which
	//* writes them back down the adapter's YAP response pipe.
	answerDialog: function(inSender, inEvent) {
		this.$.view.sendDialogResponse((inEvent && inEvent.args) || ["0"]);
	},

	// --- bookmarks / history (Jihad db8 kinds) ------------------------------
	addCurrentBookmark: function() {
		if (!this.url) { return; }
		var date = (new Date()).getTime();
		enyo.jihad.dbPut([{
			_kind: enyo.jihad.kinds.bookmarks,
			title: this._title || this.url,
			url: this.url,
			date: date,
			lastVisited: date,
			defaultEntry: false,
			visitCount: 0,
			idx: null
		}]);
	},
	//* BrowserApp.updateHistory parity: replace any prior row for this URL with a
	//* fresh one. Chained so the delete completes before the insert.
	updateHistory: function(title, url) {
		if (!url) { return; }
		var rec = {
			_kind: enyo.jihad.kinds.history,
			url: url,
			title: title || url,
			date: (new Date()).getTime()
		};
		enyo.jihad.dbDelByQuery(enyo.jihad.kinds.history,
			[{prop: "url", op: "=", val: url}], function() {
				enyo.jihad.dbPut([rec]);
			});
	},
	clearBookmarks: function() {
		enyo.jihad.dbDelByQuery(enyo.jihad.kinds.bookmarks, null);
		this.$.bookmarkList.setItems([]);
	},
	clearHistory: function() {
		enyo.jihad.dbDelByQuery(enyo.jihad.kinds.history, null);
		this.$.historyList.setItems([]);
	},

	// --- downloads (palm://com.palm.downloadmanager) ------------------------
	refreshDownloads: function() {
		var self = this;
		enyo.jihad.downloadHistory(enyo.jihad.appId(), function(resp) {
			var items = (resp && resp.items) || [];
			var list = [];
			for (var i = 0, d; (d = items[i]); i++) {
				if (d.state === "completed" && d.fileExistsOnFilesys && d.recordString) {
					try { list.push(enyo.json.parse(d.recordString)); } catch (e) { /* skip */ }
				}
			}
			self.downloads = list;
			self.$.downloadList.setDownloads(list);
		});
	},
	openDownload: function(inSender, inEvent) {
		var d = inEvent && inEvent.item;
		if (d && d.completed && !d.aborted) {
			enyo.jihad.launch({target: (d.destPath || "") + (d.destFile || "")});
		}
	},
	cancelDownload: function(inSender, inEvent) {
		var d = inEvent && inEvent.item;
		if (d && d.ticket) { enyo.jihad.cancelDownload(d.ticket); }
	},
	clearDownloads: function() {
		enyo.jihad.clearDownloads(enyo.jihad.appId());
		this.downloads = [];
		this.$.downloadList.setDownloads([]);
	},

	// --- generic info dialog ------------------------------------------------
	openDialog: function(inTitle, inMessage) {
		this.$.dialogTitle.setContent(inTitle || "");
		this.$.dialogMessage.setContent(inMessage || "");
		this.$.dialog.show();
	},
	closeDialog: function() { this.$.dialog.hide(); }
});
