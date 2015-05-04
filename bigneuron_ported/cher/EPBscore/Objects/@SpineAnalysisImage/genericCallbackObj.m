function obj=genericCallbackObj(obj,h)
global self;
global ogh;
global self;
self=struct(obj);
ogh=self.gh;
genericCallback(h);
obj=set(obj,'struct',self);