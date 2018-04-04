
## libnosync - async event loop for applications with reproducible behaviour

Simple C++14 event loop implementation designed with code reproducibility in
mind. It's meant to be used in applications split into two parts:

 - "core" part with reproducible behaviour (including dependency on time),
 - small "non-core" part without full reproducibility (doing i/o, using
separate threads, etc.)

... separated with "connectors", which are objects responsible for exchanging
messages, exposed to the "core" part using generic interfaces (which can be
easily decorated or mocked).


Author: Zbigniew Chyla (zbigniew.chyla@nokia.com)

Copyright (C) Nokia 2017,2018
