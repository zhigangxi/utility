/************
游戏逻辑处理：
逻辑处理用多线程
根据玩家id，或者场景等，把玩家消息放入固定线程
这样，对玩家逻辑处理就不会多线程
在业务逻辑中，需要访问公共数据时，就发送模块间消息
模块消息分发处理时，根据发送请求方和处理数据方，是否在同一个线程，如果同一个线程，直接回调
如果不同线程则放入队列，待处理
************/
