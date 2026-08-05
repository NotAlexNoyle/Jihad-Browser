// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// This is the browser SHELL: a Mochi-composed toolbar (back / forward / reload /
// stop + address input + menu), a load ProgressBar, the JihadWebView content
// region, and the overflow menu + parity views. Feature parity with the Enyo 1.0 app
// (bookmarks / history / downloads / find / preferences / start page /
// alert-confirm-prompt-SSL dialogs) is provided by the T-053 views + overlay dialogs
// below. NOTE: those are plain Control OVERLAYS, not mochi.Popup — a floating/modal
// mochi.Popup crashes the card on this engine (see JihadDialogs.js / PARITY.md).
//
// Contract invariant (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the UI
// drives the engine ONLY through JihadWebView's callBrowserAdapter proxy and the
// palm://net.riverstonerelay.jihadBrowserMochi/* Luna service — the identical method-name set
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
	reload: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADgAAAA4CAYAAACohjseAAAENklEQVR4nO3aW4hVVRzH8c9kZZImokZkWkgqRc14KaeESM0udqGL2FO+RGUQQfQeYVDZY1ER1Js9RDco0m6GYUaDmimBUIlkVJZGjVjRxbCHtQ+uOc6cvfY5e1mn/MLAWmvv/d//3/zXba//6RkcHPRf5oR/2oHcHBfYBZzT6mK3C5yG91rd0M0Cp2EDpre6qVsFJomDEzM5MBoLsAgXYhbOwFj04CC+wy5swwfYiD8T7d8uQRz1C5yHlbgV41vcN6H4Ow83FG0/4RU8jU/qcqiuLjoXb2Mr7tRa3EhMwB1CRN/E7Doc6zSCY7Aa92DUCPfswk7swa/FOycXfxdg6jDPXIMr8TgeKJ5ri04EzsJLwhhrZiPWYB2+TbCzBCvQH7WPwv3FtWXCP6oy7XbRfnzoaHFrcREux3PKxcFneAqXCJPSO03XezGA+e042o7AfqzHxKhtH27C9fi4HUcK3sfVQjQPRO0ThQW9f5hnWlJV4CwhSmOjtgHMwWtVX96C54XIbY3axuINzKhiqIrAMcKYiyP3rjBGUrpiVb4qbA9EbZMKH5LnjioCVxs65gZwM36pYKMqB3AVtkRtfVicaqAn8YN3LjY7shTsE7pljsgNx3RhfWysr4eFHVGDnqOeKEiN4KOGrnN3OXbiYDfujuojCmomReA8oZs0WKveCSWVF4RZthIpAlc21R+s+pIaeajqA2Wz0Whh49xgo87WuZjkT55OKIvgAkM3zmtqeu8xEUe5wEVN9XU1vPOYiaO8i/ZG5V3qmTmvFRbrTliAk4ry/lY3lgmcGZV3duJRxDM12UmirIueHpX35HQkF2UCx0XlgzkdyUW3nqolUyYwjtppOR3JRZnAfVF5Wk5HclEm8POofH5OR3JRJnBHVD4XZ2b0JQtlAjc01a/L5UguygR+hMGoviKfK3koE/i7oduqy3BxPnfqJ2UdbN5arcrhSC5SBG7DW1F9KW7J4079pB46zRZOthqb8/3CodM3Wbw6mim4LaofwhMS0m2p54vb8STuK+qT8SquwM+JNtrlVLwunOw1eFhiLjE1gnCKcBbaF7WtF85Gc4kcJ4hbGLXtEPIUf6QYqLLZ/g3L8UPUtkTIGUypYCeVqYXthVHbXtwoURzVvya+EBb7OGLzhS68rKKtViwXsrzxknRQyAZX+i5t53NpszD24khOwsvCbNtWmqtgvpDveNHQHMheIZKVT/SqjMFmZgibgL5hrm0SMkRr8XWJnbOEyKzApcNc3y6M8y/bcbITgYSJ5xHca+QZebcjKexG1x4vjNtenD3Cc4fwmHDYmzzmmulUYIM+QehSFfIGI3BYWIJW4dMObdUmsMEcR35GMqHis98LY+9ZNQhrULfABicLOffFQjecKfwQaBz+En4T86OQn98irK+bimu1kkvgv4b//ala13NcYLdzXGC38zdM28pDbDwo4wAAAABJRU5ErkJggg==",
	stop: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAB/klEQVR4nO2VsY7TQBCGx75Ag2ig4ClokGhoKHgNkCguEVS8CdTQIPEMPEaEBLpHiBJFjj3r3bXX3l3zX3FeMEvCxVYKivzV2v6934x3Zkx01lkjldxmAJASUZIkSXeED3RjxqRo+k3+Wv/L11/fmsjBTYqieC6EWPT3Znt8MyKi7Xb7zBjzNcuy+6OhAFIACTO/qOuaAYCZL2NoWGdZ9tQYkwGAlPIHgAcAkqOhAC6IiIQQCwBQSrXOOTDzPIAiWKG17rqug5Tyy2azuReCHpUlEVGe55fOOSil7BAawwCgLMtPg/cnneOMiIiZ5845aK2t9x5lWb4siuKxMYZjGICLSbB9UO89qqpCURSdEEJVVYX+k58GFkPLsnxljNFSyk4IAWOM11q/HwM72Fv75L3/BqBNkuQnEdk0Tcla+z3sNbnhh4qqUVRVhbqu0TQNpJSd9x55ni+IiJbL5Z1Twn5VoxDiAzO/7qvXxS0zFZYSEa3X6z9goUCIbvrUWguttbXWDqHjCmc4aYwxudbax6V/dXV1l+h3yyilXNu2EEK87QfD8fURT5o+s4/hWcggnBkzz9u2hXMOUkqhlHo0arSFLImIdrvdO6XU52HmkW9GRJTn+ZumaVbM/GT4/ihFv6eDEQffarV6GLyjYcPNjok2QCZlNlUnGWtn/fe6BjOqYn5X7L9uAAAAAElFTkSuQmCC",
	share: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAACiUlEQVR4nN2WvW7UQBDHZ9b2+YSOi51nQKKiTJOSjxfgBahpKFEaSsQ7pEJAw0tAGRFFgoLqpERKpEhEjrxjZ5Wzz/b+KbIXWSLnu3OChJjGXzPz2/nv7qyJ/nfjPkEAPBfbMDPudkh/wrjreZn564KYGXmePw/DcPPo6OgzM18AUMxs1wEvhTkZKcuyXTgzxnw7OTnZdD7qLmGqDRORmda6tNbCGLN/fHwc3wm0DRORXQDQWs9EBFmWIU1T2zQNjDH7aZpuzGNuA1QAPBF57yqzxhhora3WuhERMcagrmtkWfZTRDbdIBdCV5EAAH64+19VVb0OggDD4ZCttS/quv7ieR4BOJhOpyX13Gpz0vViEZGXSZJspWn6qKoqVFUFEdk6Pz8f53n+ph3TG3iTicjTsixRliW01k/Wje+UFMAQQJym6QaA0M1p03KxToVBkiT3AcRnZ2ejtYEAfFfNKyI6ZOaD09PTkdvc15L5vk/MDGaeBUHwgYgOwzD86HLcmLuz0yil7hFRzMzEzHMQWlW2++gGEcXuutA6gS4xAFTzd03TBGEYekRERVEELd/a+da9gXQlHxMRAwARked5SVmWX4mIfN9PALCTlYmIW0r0Al7baDSyRERxHH8nosfz926uVj6iVu59SikfgA9gMJlMwslkEgIIiMh3i2yl/bdqhRiPx0mXg9Z6dmugtZaJyDLzUGv9jpmn1lpWSrUlZLqS9IG1dumZ2AlkZkVXso+jKNrp8r28vCSlFAEY9gFaAJzn+aeiKJ4NBoOHIlLTgnliZvi+r8qyvGDmt51FdH2c+wCIRASLljwARFHEe3t7xfb29nSFnDdbn86/7NRfmnBd6F//bfzn7TdXVcWzva+xYwAAAABJRU5ErkJggg==",
	newtab: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAADCUlEQVR4nO1VsY7kRBB9z257Znc8zLDHIRFBBiHB3okQhAj5AhKEiC8guABICfkJwk0RIKILCViJiA0uOV2EtLtMe8f23NjjrkeA57S3O+uZWwjvSS1L5ap6Xa+7q4BX+J/BbQ6Stvq8kJDUrXcjKb5NTN8mXV8wySBpD8Awz3OR3JhIkqbTKbz3InnRm/OGBATAoii+TpLky6Zp9sysV1qScs6J5M8hhIfj8fjvtb0v7rmMeZ5/I0lt22pXmJkkaTab/SopkhT1VrjW/vz8PHPOPc6y7G5Zln+Q/FFSRNI617b7RgAikpEkA/B+mqafRlHkQgj3R6PR75JikuGm6thVd+C9P5Ok+Xz+sFeSSzg7O7vXNI3qujbv/ceXFVvjWslrbgDWBexLct03mc/nd/M8fyKp6GRPJGXHx8dJkiQTSWvlNlZ1E+E6CJKMZAvgGclVXddLM5sAyADEJFcAnh0eHq4khavxL0P4AoqiuJPn+evOuTsk2+6CDL33UwBvdNJtffS977BDCwBmduyc21+tVgRwUJYlST4A8IWZxfP5/BOSZRRFMLMbk+1CCADY399/O0kSAEBRFAghIMuykXNuBABRFO2ZWb0tzy6EEQAsFotvnXNxCGEo6cFwOBxVVfUIwKPxeJw0TfM0TdN3ukvz3wmn0+l3a4P3/vPhcLjXNM1Pk8nk+0v297YRbr003aOOJWWSBqenp28BOAAQSXpT0kDSa5Icdpg+fRUKAMxs0XWKsrP/NZvN7gNIST4hWQOoASDP8zlJmJnatt1IvpFQ/+riQgiB5Afe+49IxpKCc04hhOri4sJIvuu9H6z/SfqwbVtzzkWDwWBzO9tAFkui9/4HSSqKQsvlUsvlUnVdq65rrVYrtW2rro09X1VVSZLyPH8saSSJV2fjpjM0AIjj+KvFYvFLkiS4OgbNDCEErC+IJEiCcw51Xf8J4DOSFQBeHU9bD7mqqntpmmZlWfb6OecQx3E4OTn5rWtz18h6sUmOl4jduWVew9HRUdyd667rVpt8hVvjH9tHTdc+UNHdAAAAAElFTkSuQmCC",
	bookmarks: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAD9klEQVR4nN2VT2gjdRTH3/vNJI3NJKZIi7KHkoOwuD2IK+LBlQraw4IHkV4UFC8eRBCPnnQ9qujV9eZJaPUiqOAWuwcF0d5qFE8BPSxJmpk3md/8SebP10N/U9JsGrrS0z4IGea93/fz3vv9fm+I7nfjixICoIhIzXHlzIyL4tyznbtCAEzHFZys2d3dxfb2NjFz7nneZqvV2vQ8r2BmBQCNRoO11l+urKx0AShmLs4F2t/ftxf4LQCW67ofAUBRFJg2rfVWGUdEdKaQCSqzygA4WusnATxVrVbtPM/9SqXya6fTOdzY2JiISEBEme/7mdEt6vW6AjCZ1lyUuWLmYjAYNOr1+rtxHL/pOM6l2bh2u/3H0dHRB0SkANhERMxsAyiY+dQWnAksYf1+/wnHcb6o1WpX4zgmEfkFwC2l1JiIHrIs67rjOBvLy8tfi8hAa01EZAHIZ0EL2wiAe73ew6PR6A4A+L6/Px6Pr8yJtbTWW1EU/ZvnOVzXLcIwBACISDaZTIogCJ4rY88EEhF5nvedgX1VtgoAi8h7eZ4HruteK9f0+/1HtNaHSZLkItL1ff9DrTUAYDQaPTsNVDMwi5kLz/M2Hce5HkXRnyLyFjNnAGrmAleVUg4d75kCUF9bW7uTpum3S0tLipm52WzeyLLshSRJIgDONGPeZCBmftW2bRqPxzfX19c9ABUiyoy7OM4NqTnBEQC2bft2GIYHzWZzXUReb7Vae1rrFwEMynrmAQsASwA2kySZKKW+Nxc+n4qZEBEsy3IANIjoQSKqNhqNW+Px+GOTzDUA1urq6k+tVut3U0RxCgiAmRki8gARXUqSZJymac+0sfwRgMdNO38QkWGWZUci8gwAValUekTESql1Zs4B2CbhE7vrWgCAUqqYDZzyH2ZZ9jQzd81BYGb2mbkYjUblkC5HWLFwcANgALaIHKRpmoZheNUcjFNHutvt1uas4yAI3gZQiMhn5v1dBc3uocXMGRHt2bZtJ0nyMjMXnU7nBAiA2+12YhIpk7GZGWmavmIq/rEMP7M6I6aIiEaj0eUoiiZJkgyGw+GG8VXLNk/9KwBVIiLXdV9L07QQkb8B1MqqFwKNiGUEPgGAMAwPfN9/dDqpchqV70RkK45jL01TeJ730rTOeYBsBG3P8342n5ie1vqNMAxPDe8kSS5rrd+P4xgAMBwOP70n2BxoMwzDm+W3TUQiz/Nui8ie67q/lXMziqKx7/vvlLBztXIetHwOguD5OI6/CYLAm/m4/hMEwedBEFyZXXOWLQwoBcq7JCIrSqnHarWaNZlMdL1e/4uZ47IyZs4X6Z3bTJvmzt1FvosAswFYOzs7/2+v7nv7D6hZPMoW7YwgAAAAAElFTkSuQmCC"
};

// Dark reload/stop for INSIDE the light URL bar (the white nav icons would be
// invisible there).
enyo.JihadInlineIcons = { reload: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADgAAAA4CAYAAACohjseAAADxElEQVR4nO2ay0tVQRzHP9PLpKxVEZkakQaRpUUaQpBB0GMRFEGL2kQQIf0JUYlEraJWUcuWtewdIWb0pNIWQWGBi6S6YtENobJ+LeZcmnvuufe8Zm7d8gsDc65n5vf9nJk5Zx4qEeFf1pQ/bcC1JgH/dimlFpf6e0UDKqXqgdul7qlYQA+uF1hS6r6KBIwKBzDNkYEqoAPoBJqBZcACYDaggCzwDhgCngL9wB0R+R4xxD4iwAEgItYSsAY4B3wCJGYaA84DrRHiHDXLlrzXEthq4EYCqGLpGtBiAzBVF1VKVQMngC5gapHbhoAXwDAwjh4W87y0AqgLKLMZ2KSUOg0cFpHxxCZTtNoy4DnBLdAH7AcWRqynC3hQpK5BYGlZuyjQDowGmLkMrEnx0DYQ3NVHgbayAHpwWZ+B98B2G+PZi7GHwhdVFmh3Cuh1J3/L3Y/SFRNA1gOPfbEyQKMTQKA6YMzdBGbZhjNizvUeoBlzAOhxAXg6oOWcwRlxa4BHvtj3rAKiv3MTvjFnvVuWiL/ENyZ/2gb0v9m2lwvO8LA74O2aHhA9/cr7FJQbzvDSGxcwymrigO/6SIQyrtQdu0TIE6siv+/3WWyNeuA1RbpdnJSmBTvQr+qcLsR+ggGKs55LqzDATt/11bQBywkH4QvelUZ+SERGLMTcClxMWUcHMN3LZ0rdGAbYZORfpHGUk4ictVFPVIV10flGftilEVcKA6wx8lmXRlypInfV4igM0Gy1OS6NuFIY4AcjX+/SiCuFAb4y8stdGnGlMMBBI79UKbXQpRkXCgPs9V1vc2XElcIA76Mn2zntdWfFjUoCishX8qdV65VSa91asqso30H/1OqYCyOuFAooIk+B68ZPW5RSO9xZsivlLT5L36RUC3qPMjc5z6BPgd66s5YXvxa9GZzTBHBGohy3xViBnyJ/Jf0QmF2GfZhZwBNf7J7I5WMEmonedDUD3XIJiZ7s+zeaBoAZ1gG9gI3o7ulvyVoHcHUUbviOAA2x6kkQuI3Cw5cMsNMi3C4Kz0A+k+DkKqmBtoCWFPTbti1JnUa9twLqHQFWJ6ozhZnGgDGZS/3o/dRFEepZBBzEd95gpGfA4qQ+I30mikkpNRM4Dhyi+P7OG34fYX/xfpsL1KI3tRqKlJsATgLdIvItsUlLY2YVcAXfoUjC9BO4BDRb8WajEgO0FT21G0sA9g44YwvMShctJqXUDGAdsBHdDZvQ/whUA/wAPnoP4SV6hvQAuCsiP6x7cQH4N+m/31WreE0CVromAStdvwC9z4Jb+d27LgAAAABJRU5ErkJggg==", stop: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAACXklEQVR4nO2VsW4TQRCG/53ZQ0LCgJJHyBtgWfJd5nQOPIPLFEiJhWiQeAHnKlpEgxIXUJOWJuVhUxoRpFyZio6CDiXO7Q4Fd9LhOMnFoUHKL22zmp3v/rmZXeBWt/rXGg6H1O/3uUEoATDlWlpUBzeJK3V9aAUQkUREBgCQJImdj6v2wjCMkiT5GEVR69rQEmZE5Ekcxz97vZ7Gcbw1D63BOnEc/9jY2FAR+dbtdldUdWF5z30xAOR5bgB4AGvM/LAoihkzj0TEZFk2qkBZlhVhGHaCIDjw3j/w3sMY87XVap3u7OwYADqf+0Lbw+GQ0jT1URRtBUEw8t6fEVHgnBtMJpNR5ayCWWvJOTcaj8eDWu7mwKpkWZYVIrLNzHs16CYRHarqJ1W9X4f1+33e39/3i2BXAueh1to95xy89x7ALyK6Z61FURSNYI2Ac9BNY8xbVb0LgIjIqeqb8Xj8sgkMOD8/l0pVvxhjTvGnoWYA4L0/BIDj42O6CtYIWLkLw7DDzJ9VdcUYY4nojvfeWGvfi8hgOp2etdvt4EbAOiwIggNVbVlroaqvVfUpM5NzrmDmXRHZnk6nZ4suh7quHIvLWl9EBsy8u2BkFo7EhcASpiLymIg+lDCut/7R0RHneT6rRsY5VxCR9d4/Z+ZRr9fzaZr6RiUtbxoFsEZEKyVsr976eZ7P2u12MJlMRs65ARFZYwwAvDo5OVlN01Qvq+BClwCwvr7+QkTe1fb+SlL9syiKniVJ8r3b7T6qn7+u6ocufOuq5J1OZ7UWu7QIzea1gizlbFndyNmt/g/9Bn89SYCL2k6MAAAAAElFTkSuQmCC" };

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
			// Engine-created page (link with target / window.open) -> its own card.
			onNewPage:         "newPageRequested",
			// Engine-driven JS dialogs (T-053): presented by JihadDialogs.
			onDialogAlert:        "showAlertDialog",
			onDialogConfirm:      "showConfirmDialog",
			onDialogPrompt:       "showPromptDialog",
			onDialogSSLConfirm:   "showSSLDialog",
			onDialogUserPassword: "showLoginDialog",
			// Engine <select> dropdown -> the card-side list (Atlas model).
			onOpenSelect:         "showSelectPopup"
		},
		// Find-in-page bar (overlay below the toolbar); forwards to findInPage.
		{kind: "JihadFindBar", name: "findBar", onFind: "findRequested", onClose: "findClosed"},
		// Start page rendered as APP CHROME (not a size-limited WebView data: URL),
		// so the logo is the crisp bundled asset — matches the Enyo variant. Shown
		// over the (idle) WebView until the first navigation; hidden on loadStarted.
		{name: "startPage", classes: "jihad-startpage", components: [
			{tag: "img", name: "spLogo", classes: "jihad-sp-logo", attributes: {src: "icon-256x256.png"}},
			{content: "Jihad Browser", classes: "jihad-sp-title", allowHtml: false},
			{content: "Mochi UI \u2605 Goanna/6.9 UXP/b2594a4", classes: "jihad-sp-sub", allowHtml: false},
			{content: "Type a web address or a search in the bar above, then press Enter.",
				classes: "jihad-sp-hint", allowHtml: false}
		]},
		// Overflow menu (opened from the toolbar menu button). Items launch the
		// T-053 parity views below.
		{name: "menuPopup", classes: "jihad-menu-overlay", showing: false, ontap: "closeMenu", components: [
			{classes: "jihad-menu-box", components: [
				{classes: "jihad-menu-item", ontap: "menuNewCard",      content: "New Card"},
				{classes: "jihad-menu-item", ontap: "menuBookmarks",    content: "Bookmarks"},
				{classes: "jihad-menu-item", ontap: "menuHistory",      content: "History"},
				{classes: "jihad-menu-item", ontap: "menuDownloads",    content: "Downloads"},
				{classes: "jihad-menu-item", ontap: "menuFind",         content: "Find in Page"},
				{classes: "jihad-menu-item", ontap: "menuAddBookmark",  content: "Add Bookmark"},
				{classes: "jihad-menu-item", ontap: "menuPreferences",  content: "Preferences"}
			]}
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
		// Engine <select> popup (msgPopupMenuShow -> JihadWebView.onOpenSelect):
		// the same overlay idiom as menuPopup (mochi.Popup crashes on this engine).
		// Items are built per-popup in showSelectPopup; the box is anchored under
		// the tapped <select> from the daemon's rect (Jihad-additive JSON key).
		{name: "selectPopup", classes: "jihad-menu-overlay", showing: false,
			ontap: "selectPopupDismiss", components: [
			{name: "selectBox", classes: "jihad-menu-box jihad-select-box"}
		]},
		// Engine-driven dialog set (alert / confirm / prompt / auth / SSL).
		{kind: "JihadDialogs", name: "dialogs", onDialogAnswer: "answerDialog"},
		// Generic info dialog (page/engine errors, Share placeholder).
		{name: "dialog", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "dialogTitle",   classes: "jihad-dialog-title"},
				{name: "dialogMessage", classes: "jihad-dialog-message"},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn", ontap: "closeDialog", content: "OK"}
				]}
			]}
		]}
	],

	// Dev-loop boot marker: "@DEV@" is replaced with a per-push stamp by
	// build/webos-oe/push-card-js.sh; a stale WebAppMgr JS cache shows the old stamp.
	jihadBuildStamp: "@DEV@",

	// --- init + launch parameters -------------------------------------------
	create: function() {
		this.inherited(arguments);
		enyo.log("[JIHAD-BOOT] stamp=" + this.jihadBuildStamp);
		//* Session download records (download-manager status + history), rendered
		//* by the DownloadList view.
		this.downloads = [];
		//* Latest page title (from the engine), used when saving a bookmark/history.
		this._title = "";
		//* Chrome state initialised here (not lazily in the callbacks) so an engine
		//* callback that arrives out of order can't read an undefined guard: a
		//* loadProgress before loadStarted used to compare against `undefined` and
		//* silently freeze the bar at 0.
		this._lastProgress = 0;
		this._loading = false;
		this._lastSubmit = null;
		//* Exact URL of an inline start-page document the SHELL itself loaded (see
		//* isStartPageUrl below). Null while the start page is app chrome.
		this._startPageDocUrl = null;
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
	//* (a JSON string). A card this app opened itself (openCard below) carries its
	//* params in the query string instead — the same form Enyo 1.0's enyo.windows
	//* uses for cross-domain windows and BrowserApp.processQueryString reads — so
	//* merge those in for any key PalmSystem did not supply.
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
		p = p || {};
		var q = this.processQueryString();
		for (var k in q) {
			if (p[k] === undefined) {
				p[k] = q[k];
			}
		}
		return p;
	},
	//* Launch params carried in the URL (BrowserApp.processQueryString parity).
	//* Understands the flat `url`/`target`/`webviewId`/`query` form and the
	//* `enyoWindowParams=<json>` envelope openCard writes.
	processQueryString: function() {
		var out = {};
		var q = (window.location && window.location.search) ? window.location.search.slice(1) : "";
		if (!q) {
			return out;
		}
		var args = q.split("&");
		for (var i = 0, a, nv; (a = args[i]); i++) {
			nv = a.split("=");
			if (nv[0]) {
				out[nv[0]] = decodeURIComponent((nv[1] || "").replace(/\+/g, " "));
			}
		}
		if (out.enyoWindowParams) {
			var inner = null;
			try {
				inner = enyo.json.parse(out.enyoWindowParams);
			} catch (e) {
				inner = null;
			}
			delete out.enyoWindowParams;
			if (inner) {
				for (var k in inner) {
					if (out[k] === undefined) {
						out[k] = inner[k];
					}
				}
			}
		}
		if (out.query && out.url === undefined) {
			out.url = this.searchUrl(out.query);
		}
		return out;
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
			this.rememberStartPageDoc(this.url);
			this.$.view.setUrl(this.url);
			this.$.address.setValue(this.addressTextFor(this.url));
		}
	},

	// --- start-page URL identity (address cosmetics + history hygiene) -------
	// The start page is APP CHROME (the `startPage` overlay above), so while it
	// stands in the shell has no location and the address bar stays EMPTY —
	// exactly what the Enyo 1.0 app does (BrowserApp.startPageShown ->
	// startPage.setUrl("")). Earlier Mochi builds instead loaded the start page
	// INTO the WebView as an inline data: document; the engine then reported that
	// raw data: URL back as the location, so it landed in the address bar (device
	// report 2026-07-19) and in the db8 history kind. `startPageUrl` is the name
	// the engine itself uses for that page (BrowserPageGoanna aliases the internal
	// data: document to "about:jihad"), and `_startPageDocUrl` records the EXACT
	// URL string whenever the SHELL points the view at an inline document.
	//
	// The match is by identity, never by a "starts with data:" guess: a page may
	// legitimately navigate to a data: URL, and aliasing that to "about:jihad"
	// would let a page spoof the address bar.
	//* Canonical name of the start page (the engine's own internal about: page).
	startPageUrl: "about:jihad",
	//* Record `url` as the start-page document if the shell just loaded an inline
	//* one; any other shell navigation clears the association.
	rememberStartPageDoc: function(url) {
		this._startPageDocUrl = (url && url.slice(0, 5) === "data:") ? url : null;
	},
	//* True when `url` is the inline start-page document rather than a location.
	isStartPageUrl: function(url) {
		return Boolean(url) && url === this._startPageDocUrl;
	},
	//* Address-bar text for an engine-reported URL.
	addressTextFor: function(url) {
		if (!url) {
			return "";
		}
		return this.isStartPageUrl(url) ? this.startPageUrl : url;
	},
	//* True for URLs that must never be persisted as a history/bookmark entry:
	//* nothing loaded, the start-page document, or any inline data: document (the
	//* daemon skips global history for these too — an inline document can be
	//* megabytes and is not a revisitable location).
	isTransientUrl: function(url) {
		return !url || this.isStartPageUrl(url) || url.slice(0, 5) === "data:";
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
	//* This variant's OWN Luna service, matching the Enyo app's method-name set but not its
	//* service NAME: `palm://com.palm.browserServer/` is the stock daemon we coexist with, and
	//* calling it clears the wrong browser (see app/source/Browser.js for the measurement).
	//* Each variant owns `net.riverstonerelay.jihad-browser<variant>`, as it owns its MIME,
	//* YAP socket, upstart job and state dir.
	clearCookies: function() { new PalmServiceBridge().call('palm://net.riverstonerelay.jihadBrowserMochi/clearCookies', '{}'); },
	clearCache:   function() { new PalmServiceBridge().call('palm://net.riverstonerelay.jihadBrowserMochi/clearCache', '{}'); },

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
		this.rememberStartPageDoc(url);
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
	closeMenu: function() { this.$.menuPopup.hide(); },
	openMenu: function(inSender) {
		// Show the overflow-menu overlay (a scrim + item box; not a mochi.Popup, which
		// crashes this engine). Tapping the scrim or any item closes it.
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
			// The start-page document has no location to show (see addressTextFor).
			this.$.address.setValue(this.addressTextFor(inEvent.url));
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
	//* New card (Enyo-parity "new tab"). Enyo 2 has NO `enyo.windows` — that is an
	//* Enyo 1.0 / webOS-framework API, so the previous `enyo.windows.openWindow`
	//* call was a silent no-op in this bundled Enyo-2 app. Open the card the way
	//* enyo.windows' own agent does (windows/agent.js): window.open with the webOS
	//* `attributes=` window descriptor. Params ride in the query string, which
	//* readLaunchParams/processQueryString read back. [device-gated]
	openCard: function(params) {
		var url = "index.html";
		if (params) {
			url += "?enyoWindowParams=" + encodeURIComponent(enyo.json.stringify(params));
		}
		if (window.enyo && enyo.windows && enyo.windows.openWindow) {
			return enyo.windows.openWindow("index.html", null, params || null);
		}
		if (window.open) {
			return window.open(url, "", 'attributes={"window":"card"}');
		}
	},
	doNewCard: function() { this.openCard(null); },
	//* The engine created a page for us (target=_blank / window.open): bind it to a
	//* new card by identifier, exactly as Browser.openNewCardWithIdentifier does.
	newPageRequested: function(inSender, inEvent) {
		var id = inEvent && inEvent.identifier;
		if (id) { this.openCard({webviewId: id}); }
	},
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

	// --- engine <select> popup ----------------------------------------------
	//* Present the daemon's option list (JihadWebView.onOpenSelect). One row per
	//* item, disabled rows dimmed and inert (the daemon enforces too — the row
	//* just must LOOK the part). Reply exactly once: a pick sends its index, a
	//* scrim tap sends -1 so the daemon releases the held element.
	showSelectPopup: function(inSender, inEvent) {
		var items = (inEvent && inEvent.items) || [];
		if (!items.length) { return true; }
		this._selectPopupId = inEvent.id;
		var box = this.$.selectBox;
		box.destroyClientControls();
		for (var i = 0; i < items.length; i++) {
			var disabled = items[i].isEnabled === false;
			box.createComponent({
				classes: "jihad-menu-item" + (disabled ? " jihad-select-disabled" : ""),
				content: items[i].text || "", allowHtml: false,
				_optIndex: disabled ? -1 : i, ontap: "selectPopupPick"
			}, {owner: this});
		}
		this.$.selectPopup.setShowing(true);
		box.render();
		this.positionSelectBox(inEvent.rect);
		return true;
	},
	//* Anchor the list under the tapped box: rect is in view px (the plugin blits
	//* 1:1), so add the view node's offset in the card, clamp to the card, and
	//* flip above the box when it would run off the bottom. No rect -> leave the
	//* CSS default position (same as the overflow menu).
	positionSelectBox: function(rect) {
		var box = this.$.selectBox, node = box.hasNode(), vnode = this.$.view.hasNode();
		if (!node || !vnode || !rect || typeof rect.bottom !== "number") { return; }
		var v = vnode.getBoundingClientRect();
		var w = node.offsetWidth, h = node.offsetHeight;
		var left = Math.max(0, v.left + (rect.left + rect.right) / 2 - w / 2);
		if (left + w > window.innerWidth) { left = Math.max(0, window.innerWidth - w); }
		var top = v.top + rect.bottom;
		if (top + h > window.innerHeight) {
			top = Math.max(0, v.top + rect.top - h);
		}
		box.applyStyle("left", left + "px");
		box.applyStyle("top", top + "px");
		box.applyStyle("right", "auto");
	},
	selectPopupPick: function(inSender) {
		var idx = inSender._optIndex;
		if (idx === undefined || idx < 0) { return true; }  // disabled row: popup stays up
		var id = this._selectPopupId;
		this._selectPopupId = null;
		this.$.selectPopup.setShowing(false);
		if (id != null) { this.$.view.callBrowserAdapter("selectPopupMenuItem", [id, idx]); }
		return true;                                        // don't bubble into selectPopupDismiss
	},
	selectPopupDismiss: function() {
		var id = this._selectPopupId;
		this._selectPopupId = null;
		this.$.selectPopup.setShowing(false);
		if (id != null) { this.$.view.callBrowserAdapter("selectPopupMenuItem", [id, -1]); }
		return true;
	},

	// --- engine-driven dialogs (present via JihadDialogs) -------------------
	// Each guards its payload: these are engine-driven callbacks, and a dialog
	// raised with no message must still present (and stay answerable) rather than
	// throw on a missing event object and leave the page blocked.
	showAlertDialog:   function(inSender, inEvent) { this.$.dialogs.showAlert((inEvent && inEvent.message) || ""); },
	showConfirmDialog: function(inSender, inEvent) { this.$.dialogs.showConfirm((inEvent && inEvent.message) || ""); },
	showPromptDialog:  function(inSender, inEvent) { this.$.dialogs.showPrompt((inEvent && inEvent.message) || "", (inEvent && inEvent.defaultValue) || ""); },
	showSSLDialog:     function(inSender, inEvent) { inEvent = inEvent || {}; this.$.dialogs.showSSL(inEvent.host, inEvent.code, inEvent.certFile); },
	showLoginDialog:   function(inSender, inEvent) { this.$.dialogs.showLogin((inEvent && inEvent.message) || ""); },
	//* The user answered a dialog -> hand the string args to the WebView, which
	//* writes them back down the adapter's YAP response pipe.
	answerDialog: function(inSender, inEvent) {
		this.$.view.sendDialogResponse((inEvent && inEvent.args) || ["0"]);
	},

	// --- bookmarks / history (Jihad db8 kinds) ------------------------------
	addCurrentBookmark: function() {
		// Never bookmark the start page / an inline document (isTransientUrl).
		if (this.isTransientUrl(this.url)) { return; }
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
		// The start page and any inline data: document are chrome, not visits.
		if (this.isTransientUrl(url)) { return; }
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
		// BrowserApp.openDownloadedFile parity: completed, not aborted, not interrupted.
		if (d && d.completed && !d.aborted && !d.interrupted) {
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
