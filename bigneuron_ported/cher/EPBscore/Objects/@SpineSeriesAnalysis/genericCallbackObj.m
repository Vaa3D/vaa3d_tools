function obj=genericCallbackObj(obj,h)
global self;
global ogh;
self=struct(obj);
ogh=self.gh;
genericCallbackspine(h);
obj=set(obj,'struct',self);