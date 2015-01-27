v8like
======

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=isaacburns&url=https://github.com/flyover/v8like&title=v8like&language=JavaScript&tags=github&category=software) [![PayPal donate button](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=H9KUEZTZHHTXQ&lc=US&item_name=v8like&currency_code=USD&bn=PP-DonationsBF:btn_donate_SM.gif:NonHosted "Donate to this project using Paypal")

A V8 API layer for JavaScriptCore on iOS and OS X.

A fork of the node project including a target for building with v8like on iOS and OS X can be found at: https://github.com/flyover/node/tree/v0.10.36-release.

Notes
-----

JSC has no public API for v8::Object::[G|S]etIndexedPropertiesExternalArrayData*. JSC only allows JSObjectSetPrivate on objects created with a custom JSClassRef. So, node::Buffer must be wrapped in such an object to attach external array data.
