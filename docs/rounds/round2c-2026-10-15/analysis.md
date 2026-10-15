| # | Visit | Address | First paint | Load done | Paints | fps (avg/min) | Worst tick | RSS after | Refused | Errors | Shot |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | probe-apis | `http://192.168.1.245:8077/jihad-apis.html` | 7 s | 11 s | 11 | avg 1.8 min 0.7 n=2 | 1096 ms | 149 MB | 0 | 3 | `shot-r2-probe-apis` (75 KB) |
| 2 | probe-jsbench | `http://192.168.1.245:8077/jihad-jsbench.html` | 8 s | 9 s | 7 | avg 0.4 min 0.1 n=3 | 12371 ms | 94 MB | 0 | 0 | `shot-r2-probe-jsbench` (44 KB) |
| 3 | news-guardian | `https://www.theguardian.com/international` | 7 s | 42 s | 23 | avg 0.6 min 0.0 n=6 | 13222 ms | 209 MB | 5 | 7 | `shot-r2-news-guardian` (503 KB) |
| 4 | news-npr | `https://www.npr.org/sections/news/` | 4 s | 25 s | 23 | avg 0.7 min 0.1 n=8 | 13022 ms | 268 MB | 10 | 1 | `shot-r2-news-npr` (458 KB) |
| 5 | news-apnews | `https://apnews.com/` | 1 s | 21 s | 14 | avg 1.1 min 0.0 n=11 | 22096 ms | 249 MB | 0 | 4 | `shot-r2-news-apnews` (117 KB) |
| 6 | news-aljazeera | `https://www.aljazeera.com/` | 4 s | 23 s | 34 | avg 1.0 min 0.1 n=5 | 8564 ms | 282 MB | 5 | 0 | `shot-r2-news-aljazeera` (662 KB) |
| 7 | ref-britannica | `https://www.britannica.com/topic/Palm-Inc` | 3 s | 22 s | 12 | avg 0.6 min 0.1 n=8 | 35639 ms | 248 MB | 0 | 4 | `shot-r2-ref-britannica` (120 KB) |
| 8 | ref-osm | `https://www.openstreetmap.org/#map=12/37.7749/-122.4194` | 8 s | 22 s | 18 | avg 0.9 min 0.1 n=5 | 3682 ms | 210 MB | 0 | 4 | `shot-r2-ref-osm` (473 KB) |
| 9 | ref-wikihow | `https://www.wikihow.com/Use-a-Tablet` | 6 s | none | 5 | avg 0.2 min 0.1 n=2 | 218 ms | 189 MB | 0 | 0 | `shot-r2-ref-wikihow` (32 KB) |
| 10 | shop-amazon | `https://www.amazon.com/s?k=hp+touchpad` | 10 s | 49 s | 30 | avg 0.7 min 0.0 n=12 | 60165 ms | 223 MB | 2 | 2 | `shot-r2-shop-amazon` (207 KB) |
| 11 | shop-ebay | `https://www.ebay.com/sch/i.html?_nkw=hp+touchpad` | 3 s | 10 s | 11 | avg 0.7 min 0.1 n=5 | 4505 ms | 211 MB | 1 | 7 | `shot-r2-shop-ebay` (128 KB) |
| 12 | shop-etsy | `https://www.etsy.com/search?q=webos` | 8 s | 45 s | 55 | avg 1.4 min 0.0 n=17 | 6781 ms | 239 MB | 16 | 53 | `shot-r2-shop-etsy` (644 KB) |
| 13 | social-oldreddit | `https://old.reddit.com/r/webos/` | 4 s | 39 s | 13 | avg 0.6 min 0.0 n=6 | 17071 ms | 273 MB | 8 | 1 | `shot-r2-social-oldreddit` (51 KB) |
| 14 | social-reddit | `https://www.reddit.com/r/webos/` | 7 s | none | 16 | avg 0.5 min 0.0 n=6 | 28973 ms | 433 MB | 4 | 2 | `shot-r2-social-reddit` (294 KB) |
| 15 | social-bsky | `https://bsky.app/profile/mozilla.org` | 1 s | 38 s | 15 | avg 0.6 min 0.1 n=5 | 30152 ms | 564 MB | 2 | 8 | `shot-r2-social-bsky` (34 KB) |
| 16 | video-youtube | `https://www.youtube.com/watch?v=dQw4w9WgXcQ` | 1 s | 53 s | 20 | avg 0.5 min 0.1 n=12 | 13073 ms | 589 MB | 11 | 1 | `shot-r2-video-youtube` (297 KB) |
| 17 | video-vimeo | `https://vimeo.com/channels/staffpicks` | 7 s | none | 12 | avg 0.5 min 0.0 n=6 | 17412 ms | 526 MB | 8 | 0 | `shot-r2-video-vimeo` (55 KB) |
| 18 | dev-stackoverflow | `https://stackoverflow.com/questions/tagged/javascript` | 19 s | 52 s | 22 | avg 0.5 min 0.0 n=6 | 25309 ms | 556 MB | 4 | 10 | `shot-r2-dev-stackoverflow` (220 KB) |
| 19 | dev-gitlab | `https://gitlab.com/explore` | 1 s | none | 11 | avg 0.3 min 0.0 n=5 | 44773 ms | 551 MB | 0 | 4 | `shot-r2-dev-gitlab` (32 KB) |
| 20 | dev-npm | `https://www.npmjs.com/package/express` | 1 s | 24 s | 14 | avg 0.7 min 0.1 n=10 | 23113 ms | 572 MB | 0 | 5 | `shot-r2-dev-npm` (120 KB) |
| 21 | webos-ports | `https://www.webos-ports.org/` | 7 s | none | 34 | avg 1.3 min 0.4 n=6 | 10812 ms | 519 MB | 0 | 1 | `shot-r2-webos-ports` (205 KB) |
| 22 | webos-nation | `https://www.webosnation.com/` | 7 s | none | 9 | avg 0.3 min 0.2 n=4 | 18204 ms | 518 MB | 0 | 0 | `shot-r2-webos-nation` (98 KB) |
| 23 | search-startpage | `https://www.startpage.com/do/search?q=webos+touchpad` | 1 s | 7 s | 10 | avg 18.7 min 0.1 n=28 | 14822 ms | 520 MB | 4 | 2 | `shot-r2-search-startpage` (41 KB) |
| 24 | search-bing | `https://www.bing.com/search?q=webos+touchpad` | 3 s | 9 s | 17 | avg 1.0 min 0.1 n=7 | 3308 ms | 502 MB | 3 | 1 | `shot-r2-search-bing` (189 KB) |
| 25 | search-google | `https://www.google.com/search?q=webos+touchpad` | 7 s | 10 s | 8 | avg 0.3 min 0.1 n=2 | 999 ms | 495 MB | 6 | 0 | `shot-r2-search-google` (180 KB) |
| 26 | app-weather | `https://weather.com/weather/today/l/San+Francisco+CA` | 3 s | 32 s | 10 | avg 0.4 min 0.1 n=5 | 10854 ms | 533 MB | 4 | 0 | `shot-r2-app-weather` (28 KB) |
| 27 | app-medium | `https://medium.com/topic/technology` | 3 s | 16 s | 12 | avg 0.8 min 0.0 n=9 | 27987 ms | 571 MB | 0 | 7 | `shot-r2-app-medium` (118 KB) |
| 28 | app-substack | `https://substack.com/discover` | 40 s | none | 9 | avg 0.5 min 0.2 n=5 | 55081 ms | 581 MB | 1 | 4 | `shot-r2-app-substack` (47 KB) |
| 29 | app-craigslist | `https://sfbay.craigslist.org/` | 7 s | none | 19 | avg 0.6 min 0.1 n=7 | 16039 ms | 558 MB | 2 | 0 | `shot-r2-app-craigslist` (70 KB) |
| 30 | app-imdb | `https://www.imdb.com/title/tt0111161/` | 7 s | 14 s | 19 | avg 0.4 min 0.1 n=8 | 10443 ms | 548 MB | 1 | 3 | `shot-r2-app-imdb` (530 KB) |
| 31 | test-html5test | `https://html5test.co/` | 1 s | 35 s | 28 | avg 2.2 min 0.1 n=6 | 40875 ms | 514 MB | 0 | 9 | `shot-r2-test-html5test` (144 KB) |
| 32 | test-acid3 | `https://acid3.acidtests.org/` | 6 s | none | 2 | avg 0.2 min 0.2 n=1 | 284 ms | 503 MB | 0 | 1 | `shot-r2-test-acid3` (55 KB) |

Session memory: first visit 149 MB, last visit 503 MB; 32 visits.

## Issues by threshold

- **probe-apis**: frame rate fell to 0.7 fps; a 1096 ms tick (the main thread stalled); 3 page error line(s)
  - `1 http://192.168.1.245:8077/jihad-apis.html:65 [JavaScript Warning: "Error: WebGL: Refused to create native OpenGL context because of blacklist entry: " {file: "http://192.168.1.245:8077/jihad-apis.ht`
  - `1 http://192.168.1.245:8077/jihad-apis.html:65 [JavaScript Warning: "Error: WebGL: Failed to create WebGL context: WebGL creation failed:`
  - `1 http://192.168.1.245:8077/jihad-apis.html:65 [JavaScript Warning: "Error: WebGL: Disallowing antialiased backbuffers due to blacklisting." {file: "http://192.168.1.245:8077/jihad-apis.html" line: 65`
- **probe-jsbench**: frame rate fell to 0.1 fps; a 12371 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog)
- **news-guardian**: load-done after 42 s; frame rate fell to 0.0 fps; a 13222 ms tick (the main thread stalled); RSS +114 MB over the previous visit; 7 page error line(s)
  - `5 https://assets.guim.co.uk/assets/index.client.web.8c90b8b14f670ff82d94.js:1 [JavaScript Error: "TypeError: NetworkError when attempting to fetch resource." {file: "https://assets.guim.co.uk/assets/i`
  - `1 (no source):0 Security Error: Content at https://www.theguardian.com/international may not load data from https://uploads.guim.co.uk/2026/10/15/151026_TiF_Latest_Front_Loop--12a59a82-64b4-44d6-96b6-`
  - `1 (no source):0 Security Error: Content at https://www.theguardian.com/international may not load data from https://uploads.guim.co.uk/2026/10/13/Dream_of_birds_loop--6a3918a9-bab5-4d53-861a-b8096a854`
- **news-npr**: frame rate fell to 0.1 fps; a 13022 ms tick (the main thread stalled); RSS +59 MB over the previous visit; 1 page error line(s)
  - `3 https://www.npr.org/sections/news/:0 [JavaScript Warning: "Unexpected value .8s parsing dur attribute." {file: "https://www.npr.org/sections/news/" line: 0}]`
  - `1 https://www.npr.org/sections/news/:76 [JavaScript Warning: "Use of setAttributeNode() is deprecated. Use setAttribute() instead." {file: "https://www.npr.org/sections/news/" line: 76}]`
  - `1 https://www.npr.org/sections/news/:76 [JavaScript Warning: "Use of document.createAttribute() is deprecated. Use element.setAttribute() instead." {file: "https://www.npr.org/sections/news/" line: 76`
  - `1 https://www.npr.org/sections/news/:0 [JavaScript Warning: "Unexpected value .3s parsing begin attribute." {file: "https://www.npr.org/sections/news/" line: 0}]`
- **news-apnews**: frame rate fell to 0.0 fps; a 22096 ms tick (the main thread stalled); 4 page error line(s)
  - `1 https://www.npr.org/sections/news/:0 [JavaScript Warning: "Cannot play media. No decoders for requested formats: audio/mpeg, audio/mpeg" {file: "https://www.npr.org/sections/news/" line: 0}]`
- **news-aljazeera**: frame rate fell to 0.1 fps; a 8564 ms tick (the main thread stalled)
- **ref-britannica**: frame rate fell to 0.1 fps; a 35639 ms tick (the main thread stalled); 4 page error line(s)
- **ref-osm**: frame rate fell to 0.1 fps; a 3682 ms tick (the main thread stalled); 4 page error line(s)
  - `1 https://matomo.openstreetmap.org/matomo.js:84 [JavaScript Warning: "Error: WebGL: Refused to create native OpenGL context because of blacklist entry: " {file: "https://matomo.openstreetmap.org/matom`
  - `1 https://matomo.openstreetmap.org/matomo.js:84 [JavaScript Warning: "Error: WebGL: Failed to create WebGL context: WebGL creation failed:`
  - `1 https://matomo.openstreetmap.org/matomo.js:84 [JavaScript Warning: "Error: WebGL: Disallowing antialiased backbuffers due to blacklisting." {file: "https://matomo.openstreetmap.org/matomo.js" line: `
- **ref-wikihow**: no load-done for the address within the dwell; frame rate fell to 0.1 fps; a near-uniform screenshot (blank or a dialog)
- **shop-amazon**: load-done after 49 s; frame rate fell to 0.0 fps; a 60165 ms tick (the main thread stalled); 2 page error line(s)
  - `2 https://www.amazon.com/s?k=hp+touchpad:7035 [JavaScript Warning: "Ignoring unsupported entryTypes: largest-contentful-paint." {file: "https://www.amazon.com/s?k=hp+touchpad" line: 7035}]`
  - `1 https://www.amazon.com/s?k=hp+touchpad:7035 [JavaScript Warning: "Ignoring unsupported entryTypes: longtask." {file: "https://www.amazon.com/s?k=hp+touchpad" line: 7035}]`
  - `1 https://www.amazon.com/s?k=hp+touchpad:7035 [JavaScript Warning: "Ignoring unsupported entryTypes: layout-shift." {file: "https://www.amazon.com/s?k=hp+touchpad" line: 7035}]`
  - `1 https://www.amazon.com/s?k=hp+touchpad:7035 [JavaScript Warning: "Ignoring unsupported entryTypes: first-input." {file: "https://www.amazon.com/s?k=hp+touchpad" line: 7035}]`
- **shop-ebay**: frame rate fell to 0.1 fps; a 4505 ms tick (the main thread stalled); 7 page error line(s)
  - `2 https://www.ebay.com/-JM61y/YG/hT/b0UW/PqK8Wix9nh/hpuJbrO1Ni/c0MRHgE/Chs/cGVJGQnET?v=632433d8-f14a-44de-c6ef-383f550a172e:1 [JavaScript Warning: "Error: WebGL: Refused to create native OpenGL contex`
  - `2 https://www.ebay.com/-JM61y/YG/hT/b0UW/PqK8Wix9nh/hpuJbrO1Ni/c0MRHgE/Chs/cGVJGQnET?v=632433d8-f14a-44de-c6ef-383f550a172e:1 [JavaScript Warning: "Error: WebGL: Failed to create WebGL context: WebGL `
  - `2 https://www.ebay.com/-JM61y/YG/hT/b0UW/PqK8Wix9nh/hpuJbrO1Ni/c0MRHgE/Chs/cGVJGQnET?v=632433d8-f14a-44de-c6ef-383f550a172e:1 [JavaScript Warning: "Error: WebGL: Disallowing antialiased backbuffers du`
  - `1 https://www.ebay.com/sch/i.html?_nkw=hp+touchpad:0 [JavaScript Error: "The buffer passed to decodeAudioData contains an unknown content type." {file: "https://www.ebay.com/sch/i.html?_nkw=hp+touchpa`
- **shop-etsy**: load-done after 45 s; frame rate fell to 0.0 fps; a 6781 ms tick (the main thread stalled); 53 page error line(s)
  - `48 https://www.etsy.com/ac/viteEightB/js/en-US/collage/web-components/index.7WGKdbXz.js:68 [JavaScript Error: "TypeError: Element of DocumentOrShadowRoot.adoptedStyleSheets does not implement interfac`
  - `4 https://www.etsy.com/search?q=webos:0 [JavaScript Warning: "All candidate resources failed to load. Media load paused." {file: "https://www.etsy.com/search?q=webos" line: 0}]`
  - `1 https://www.etsy.com/search?q=webos:43 [JavaScript Warning: "Ignoring unsupported entryTypes: element, longtask, paint, layout-shift." {file: "https://www.etsy.com/search?q=webos" line: 43}]`
  - `1 https://www.etsy.com/search?q=webos:0 [JavaScript Warning: "The video on this page can’t be played. Your system may not have the required video codecs for: video/mp4, video/mp4, video/mp4, video/mp4`
- **social-oldreddit**: load-done after 39 s; frame rate fell to 0.0 fps; a 17071 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog); 1 page error line(s)
  - `1 https://www.redditstatic.com/shreddit/polyfills/es-module-shims.0ad448105a7238e685087a02f870c658.js:0 [JavaScript Warning: "Successfully compiled asm.js code (total compilation time 122ms; unable to`
  - `1 https://www.gstatic.com/recaptcha/releases/4EDjxXf4knsx6UBM1L2V6cv8/recaptcha__en.js:300 [JavaScript Warning: "Ignoring unsupported entryTypes: longtask." {file: "https://www.gstatic.com/recaptcha/r`
  - `1 data:text/javascript,let%20navPromise%3Dnull%3Basync%20function%20getNavigation()%7Breturn%20navPromise%7C%7C(navPromise%3D_getNavigation())%2CnavPromise%7Dasync%20function%20_getNavigation()%7Bif(!`
- **social-reddit**: no load-done for the address within the dwell; frame rate fell to 0.0 fps; a 28973 ms tick (the main thread stalled); RSS +160 MB over the previous visit; 2 page error line(s)
  - `2 (no source):0 [JavaScript Error: "TypeError: NetworkError when attempting to fetch resource."]`
- **social-bsky**: load-done after 38 s; frame rate fell to 0.1 fps; a 30152 ms tick (the main thread stalled); RSS +130 MB over the previous visit; a near-uniform screenshot (blank or a dialog); 8 page error line(s)
  - `6 https://www.redditstatic.com/js/concat?prefix=en-US/&chunks=BvOWmaNPEJ,Dyg3q1gB6W,cO_SguL2TR:46 [JavaScript Error: "NS_ERROR_NOT_INITIALIZED: " {file: "https://www.redditstatic.com/js/concat?prefix=`
  - `2 (no source):0 [JavaScript Error: "TypeError: NetworkError when attempting to fetch resource."]`
  - `1 https://www.redditstatic.com/shreddit/polyfills/es-module-shims.0ad448105a7238e685087a02f870c658.js:0 [JavaScript Warning: "Successfully compiled asm.js code (loaded from cache in 20798ms)" {file: "`
- **video-youtube**: load-done after 53 s; frame rate fell to 0.1 fps; a 13073 ms tick (the main thread stalled); 1 page error line(s)
  - `1 https://web-cdn.bsky.app/static/_expo/static/js/web/index-81076605df5d2c364f6c66b563d34bf0.js:372 [JavaScript Error: "NS_ERROR_NOT_INITIALIZED: " {file: "https://web-cdn.bsky.app/static/_expo/static`
- **video-vimeo**: no load-done for the address within the dwell; frame rate fell to 0.0 fps; a 17412 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog)
  - `1 https://www.youtube.com/s/player/0229cd64/player_es6.vflset/en_US/base.js:1477 [JavaScript Warning: "An IndexedDB transaction that was not yet complete has been aborted due to page navigation." {fil`
- **dev-stackoverflow**: load-done after 52 s; frame rate fell to 0.0 fps; a 25309 ms tick (the main thread stalled); 10 page error line(s)
  - `5 https://f.vimeocdn.com/next-server/vimeo-next/_next/static/chunks/261zsg23zpc8i.js:6 [JavaScript Error: "NS_ERROR_NOT_INITIALIZED: " {file: "https://f.vimeocdn.com/next-server/vimeo-next/_next/stati`
  - `1 https://f.vimeocdn.com/next-server/vimeo-next/_next/static/chunks/261zsg23zpc8i.js:6 [JavaScript Error: "TypeError: NetworkError when attempting to fetch resource." {file: "https://f.vimeocdn.com/ne`
- **dev-gitlab**: no load-done for the address within the dwell; frame rate fell to 0.0 fps; a 44773 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog); 4 page error line(s)
  - `3 (no source):0 [JavaScript Error: "https://gitlab.com/explore : Unable to run script because scripts are blocked internally."]`
  - `1 (no source):0 [JavaScript Error: "https://gitlab.com/explore/projects/explore : Unable to run script because scripts are blocked internally."]`
- **dev-npm**: frame rate fell to 0.1 fps; a 23113 ms tick (the main thread stalled); 5 page error line(s)
  - `1 (no source):0 [JavaScript Error: "TypeError: NetworkError when attempting to fetch resource."]`
- **webos-ports**: no load-done for the address within the dwell; frame rate fell to 0.4 fps; a 10812 ms tick (the main thread stalled); 1 page error line(s)
- **webos-nation**: no load-done for the address within the dwell; frame rate fell to 0.2 fps; a 18204 ms tick (the main thread stalled)
- **search-startpage**: frame rate fell to 0.1 fps; a 14822 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog); 2 page error line(s)
  - `1 https://www.techradar.com/:3992 [JavaScript Error: "TypeError: window.matchMedia(...) is null" {file: "https://www.techradar.com/" line: 3992}]`
  - `1 https://www.techradar.com/:1 [JavaScript Error: "ReferenceError: handleHeaderError is not defined" {file: "https://www.techradar.com/" line: 1}]`
- **search-bing**: frame rate fell to 0.1 fps; a 3308 ms tick (the main thread stalled); 1 page error line(s)
  - `1 https://r.bing.com/rp/DWZSY9SRnkHaYWjJ109REMim90A.br.js:0 [JavaScript Warning: "Successfully compiled asm.js code (total compilation time 258ms; stored in cache)" {file: "https://r.bing.com/rp/DWZSY`
- **search-google**: frame rate fell to 0.1 fps; a 999 ms tick (the main thread stalled)
- **app-weather**: load-done after 32 s; frame rate fell to 0.1 fps; a 10854 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog)
- **app-medium**: frame rate fell to 0.0 fps; a 27987 ms tick (the main thread stalled); 7 page error line(s)
  - `2 https://medium.com/favicon.ico:0 [JavaScript Error: "Image corrupt or truncated." {file: "https://medium.com/favicon.ico" line: 0}]`
  - `1 https://weather.com/weather/today/l/San+Francisco+CA:6 [JavaScript Error: "NS_ERROR_NOT_INITIALIZED: " {file: "https://weather.com/weather/today/l/San+Francisco+CA" line: 6}]`
- **app-substack**: no load-done for the address within the dwell; frame rate fell to 0.2 fps; a 55081 ms tick (the main thread stalled); a near-uniform screenshot (blank or a dialog); 4 page error line(s)
  - `1 (no source):0 [JavaScript Error: "downloadable font: rejected by sanitizer (font-family: "Lucida Console" style:normal weight:normal stretch:normal src index:0) source: (invalid URI)"]`
  - `1 (no source):0 [JavaScript Error: "downloadable font: font load failed (font-family: "Lucida Console" style:normal weight:normal stretch:normal src index:0) source: (invalid URI)"]`
  - `1 (no source):0 [JavaScript Error: "downloadable font: Size of decompressed WOFF 2.0 is less than compressed size (font-family: "Lucida Console" style:normal weight:normal stretch:normal src index:0) `
  - `1 (no source):0 Security Error: Content at https://challenges.cloudflare.com/cdn-cgi/challenge-platform/h/b/turnstile/f/av0/rch/dum50/0x4AAAAAAADnPIDROrmt1Wwj/light/fbE/new/normal?lang=auto may not lo`
- **app-craigslist**: no load-done for the address within the dwell; frame rate fell to 0.1 fps; a 16039 ms tick (the main thread stalled)
- **app-imdb**: frame rate fell to 0.1 fps; a 10443 ms tick (the main thread stalled); 3 page error line(s)
  - `1 https://fb423e1ef94f.6277d64d.us-east-1.token.awswaf.com/fb423e1ef94f/c3382d439950/916d943f6a58/challenge.js:1 [JavaScript Warning: "Error: WebGL: Refused to create native OpenGL context because of `
  - `1 https://fb423e1ef94f.6277d64d.us-east-1.token.awswaf.com/fb423e1ef94f/c3382d439950/916d943f6a58/challenge.js:1 [JavaScript Warning: "Error: WebGL: Failed to create WebGL context: WebGL creation fail`
  - `1 https://fb423e1ef94f.6277d64d.us-east-1.token.awswaf.com/fb423e1ef94f/c3382d439950/916d943f6a58/challenge.js:1 [JavaScript Warning: "Error: WebGL: Disallowing antialiased backbuffers due to blacklis`
- **test-html5test**: load-done after 35 s; frame rate fell to 0.1 fps; a 40875 ms tick (the main thread stalled); 9 page error line(s)
  - `2 https://www.imdb.com/title/tt0111161/:2619 [JavaScript Warning: "Ignoring unsupported entryTypes: largest-contentful-paint." {file: "https://www.imdb.com/title/tt0111161/" line: 2619}]`
  - `1 https://www.imdb.com/title/tt0111161/:2619 [JavaScript Warning: "Ignoring unsupported entryTypes: longtask." {file: "https://www.imdb.com/title/tt0111161/" line: 2619}]`
  - `1 https://www.imdb.com/title/tt0111161/:2619 [JavaScript Warning: "Ignoring unsupported entryTypes: layout-shift." {file: "https://www.imdb.com/title/tt0111161/" line: 2619}]`
  - `1 https://www.imdb.com/title/tt0111161/:2619 [JavaScript Warning: "Ignoring unsupported entryTypes: first-input." {file: "https://www.imdb.com/title/tt0111161/" line: 2619}]`
- **test-acid3**: no load-done for the address within the dwell; frame rate fell to 0.2 fps; a near-uniform screenshot (blank or a dialog); 1 page error line(s)
  - `1 (no source):0 [JavaScript Error: "acid3.acidtests.org:443 uses an invalid security certificate.`
