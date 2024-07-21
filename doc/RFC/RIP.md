# RIP

 This memo is intended to do the following things:     

- Document a protocol and algorithms that are currently in  wide use for routing, but which have never been formally  documented.     
- Specify some improvements in the algorithms which will improve stability of the routes in large networks.  These improvements do not introduce any incompatibility with  existing implementations.  They are to be incorporated into  all implementations of this protocol.
-  Suggest some optional features to allow greater configurability and control.  These features were developed  specifically to solve problems that have shown up in actual use by the **NSFnet** community.  However, they should have more  general utility.

这份备忘录旨在完成以下几件事情：

- 记录一种当前广泛用于路由的协议和算法，但这些协议和算法从未被正式记录过。
- 提出一些算法改进，这些改进将提高大规模网络中路由的稳定性。这些改进不会与现有实现引入任何不兼容性。它们将被纳入该协议的所有实现中。
- 提出一些可选功能，以提供更高的配置灵活性和控制能力。这些功能专门为解决**NSFnet**社区在实际使用中遇到的问题而开发。然而，它们应该具有更广泛的实用性。

这里描述的路由信息协议（RIP）大致基于与4.3 Berkeley软件分发版一起分发的程序“routed”。然而，还有其他几个被认为是同一协议的实现。不幸的是，这些不同的实现细节上存在分歧。这里的规范代表了从各种实现中结合的特性。我们相信，根据本文档设计的程序将与“routed”以及我们所知道的所有其他RIP实现互操作。

请注意，此描述在何时应增加度量值【metrics】方面采用了与大多数现有实现不同的观点。通过对本地网络使用的度量值进行相应的更改，我们保留了与其他现有实现的兼容性。有关此问题的详细信息，请参见第3.6节。

##  Introduction   

This memo describes one protocol in a series of routing protocols   based on the Bellman-Ford (or distance vector) algorithm.  This   algorithm has been used for routing computations in computer networks   since the early days of the ARPANET.  The particular packet formats   and protocol described here are based on the program "routed", which   is included with the Berkeley distribution of Unix.  It has become a   de facto standard for exchange of routing information among gateways   and hosts.  It is implemented for this purpose by most commercial   vendors of IP gateways.  Note, however, that many of these vendors   have their own protocols which are used among their own gateways.   This protocol is most useful as an "interior gateway protocol".  In a   nationwide network such as the current Internet, it is very unlikely   that a single routing protocol will used for the whole network.   Rather, the network will be organized as a collection of "autonomous   systems".  An autonomous system will in general be administered by a   single entity, or at least will have some reasonable degree of   technical and administrative control.  Each autonomous system will   have its own routing technology.  This may well be different for   different autonomous systems.  The routing protocol used within an   autonomous system is referred to as an interior gateway protocol, or   "IGP".  A separate protocol is used to interface among the autonomous  systems.  The earliest such protocol, still used in the Internet, is   "EGP" (exterior gateway protocol).  Such protocols are now usually   referred to as inter-AS routing protocols.  RIP was designed to work   with moderate-size networks using reasonably homogeneous technology.   Thus it is suitable as an IGP for many campuses and for regional   networks using serial lines whose speeds do not vary widely.  It is   not intended for use in more complex environments.  For more   information on the context into which RIP is expected to fit, see   Braden and Postel [3].   RIP is one of a class of algorithms known as "distance vector   algorithms".  The earliest description of this class of algorithms   known to the author is in Ford and Fulkerson [6].  Because of this,   they are sometimes known as Ford-Fulkerson algorithms.  The term   Bellman-Ford is also used.  It comes from the fact that the   formulation is based on Bellman’s equation, the basis of "dynamic   programming".  (For a standard introduction to this area, see [1].)   The presentation in this document is closely based on [2].  This text   contains an introduction to the mathematics of routing algorithms.   It describes and justifies several variants of the algorithm   presented here, as well as a number of other related algorithms.  The   basic algorithms described in this protocol were used in computer   routing as early as 1969 in the ARPANET.  However, the specific   ancestry of this protocol is within the Xerox network protocols.  The   PUP protocols (see [4]) used the Gateway Information Protocol to   exchange routing information.  A somewhat updated version of this   protocol was adopted for the Xerox Network Systems (XNS)   architecture, with the name Routing Information Protocol.  (See [7].)   Berkeley’s routed is largely the same as the Routing Information   Protocol, with XNS addresses replaced by a more general address   format capable of handling IP and other types of address, and with   routing updates limited to one every 30 seconds.  Because of this   similarity, the term Routing Information Protocol (or just RIP) is   used to refer to both the XNS protocol and the protocol used by   routed.   RIP is intended for use within the IP-based Internet.  The Internet   is organized into a number of networks connected by gateways.  The   networks may be either point-to-point links or more complex networks   such as Ethernet or the ARPANET.  Hosts and gateways are presented   with IP datagrams addressed to some host.  Routing is the method by   which the host or gateway decides where to send the datagram.  It may   be able to send the datagram directly to the destination, if that   destination is on one of the networks that are directly connected to   the host or gateway.  However, the interesting case is when the   destination is not directly reachable.  In this case, the host or   gateway attempts to send the datagram to a gateway that is nearer the   destination.  The goal of a routing protocol is very simple: It is to   supply the information that is needed to do routing

这份备忘录描述了一系列基于Bellman-Ford（或距离向量）算法的路由协议中的一个协议。自ARPANET初期以来，这种算法已被用于计算机网络中的路由计算。这里描述的特定数据包格式和协议基于Unix的Berkeley发行版中包含的程序“routed”。它已成为网关和主机之间交换路由信息的事实标准。大多数IP网关的商业供应商为此目的实现了它。然而，需要注意的是，许多这些供应商都有用于其自身网关之间的协议。这个协议作为“内部网关协议”最为有用。

------

在像当前互联网这样的全国性网络中，整个网络不太可能使用单一的路由协议。相反，网络将被组织为一组“自治系统”。自治系统通常由单个实体管理，或至少具有一定程度的技术和行政控制。每个自治系统将有自己的路由技术。这可能对于不同的自治系统来说是不同的。用于自治系统内的路由协议被称为内部网关协议，或“IGP”。用于自治系统之间接口的协议是单独的。最早的这种协议，仍然在互联网中使用，是“EGP”（外部网关协议）。这种协议现在通常被称为自治系统间路由协议。

------

RIP被设计用于使用相对均匀技术的中等规模网络。因此，它适合作为许多校园和使用串行线路的区域网络的IGP，这些线路的速度变化不大。它并不适用于更复杂的环境。有关RIP预期适用环境的更多信息，请参见Braden和Postel [3]。

------

RIP是一类称为“距离向量算法”的算法之一。作者所知的这种算法类的最早描述见于Ford和Fulkerson [6]。因此，它们有时被称为Ford-Fulkerson算法。Bellman-Ford这个术语也被使用。它源于基于Bellman方程的公式，这是“动态规划”的基础。（有关该领域的标准介绍，请参见[1]。）本文档中的介绍紧密基于[2]。该文本包含对路由算法数学的介绍。它描述并证明了本文档中提出的算法的几个变体以及其他一些相关算法。

------

在ARPANET中，早在1969年就使用了本文描述的基本算法。然而，这个协议的具体来源在于Xerox网络协议。PUP协议（见[4]）使用网关信息协议来交换路由信息。这个协议的稍更新版本被采纳为Xerox网络系统（XNS）架构，名为路由信息协议。（见[7]。）Berkeley的routed在很大程度上与路由信息协议相同，只是XNS地址被一种更通用的地址格式取代，能够处理IP和其他类型的地址，并且路由更新限制为每30秒一次。由于这种相似性，路由信息协议（或简称RIP）这个术语被用来指代XNS协议和routed使用的协议。

------

RIP旨在用于基于IP的互联网。互联网由许多通过网关连接的网络组成。这些网络可能是点对点链接，也可能是更复杂的网络，如以太网或ARPANET。主机和网关接收的IP数据报是发送到某个主机的。路由是主机或网关决定将数据报发送到何处的方法。如果目的地在直接连接到主机或网关的网络之一上，它可能能够将数据报直接发送到目的地。然而，更有趣的情况是目的地无法直接到达。在这种情况下，主机或网关尝试将数据报发送到离目的地更近的网关。路由协议的目标非常简单：它是提供进行路由所需的信息。

[^1]: Bellman, R. E., "Dynamic Programming", Princeton University       Press, Princeton, N.J., 1957.

### 1.1. 协议的局限性

本协议不解决所有的路由问题。如上所述，是被设计为适合中等规模网络做为IGP使用。另外，还要注意一下限制：

\- 协议限定网络最大路径为15跳。设计者认为这一协议不适合于大型网络。注意，这一限定声明是在假设经过每个网络的成本(cost)为1，正如RIP一般配置的情况。如果管理员选择了较大的成本,15的上限可能成为问题。

\- 协议使用“记数到无穷大”表示一些特殊情况(将在下一节中说明)。如果系统中有几百个网络，路由回路(routing loop)将会充斥其间。解决路由回路或需要很多时间(如果路由更新的频率受到限制)，或需要很多带宽(如果每次改变都发送更新)。这样在回路被纠正前会消耗很多带宽。我们相信，在现实情况下，这不会产生问题(除非是低速线路)。即使这样，这个问题还需要被认真对待，而且使用多种预防措施来避免在大多数情况下出现这样的问题。

\- 协议使用固定的“尺度(metrics)”来描述不同的路径。当路径选择需要基于实时参数，如：延迟、可靠性、负载时是不适合的。使用这类实时参数会明显产生不稳定性。

### 1.2. 本文档的组成

本文档的主体内容分为两部分，占据下面两章：
2 从概念上了解距离向量算法的发展和原理。
3 实际协议的描述

这两章大致按这样的范围叙述。第2章给出了算法大致的数学模型，一开始先描述简单算法，然后再逐渐细致，呈“螺旋上升”的方法。第3章描述实际的协议，将第2章的内容具体化。通过第3章的描述，就可以实现整个RIP。

2. 距离向量算法

路由的任务就是找到一条从源到目标的路径。在IP“Catenet model”中，这被简化为寻找网络间的网关。当通讯处于一个网络或子网时，由该特定的网络来解决路由问题。如以太网和ARPANET都定义了方法，使在一个网络中，任何源能够和指定的目标通讯。当源和目标不在同一网段时，IP路由才变的必要。这时，通讯必须通过连接网络的网关。当网络不邻接时，通讯将通过一系列用网关相连的网络。当通讯到达与目标处于同一网络的网关后，将使用该网络自身的方法到达目标。

在本章中，术语“网络(network)”包含单一广播网络(如以太网)、点对点线路或ARPANET。其关键点是网络被作为IP的单一实体。即便在不需要路由(如点对点线路)，或路由被设置为对IP透明的情况，都允许IP将整个网络作为是一个完全连接的系统(如以太网或ARPANET)。这里的术语“网络”和在讨论IP地址时使用的“网络”有所不同。在讨论IP地址时，一个网络可能被分为多个“子网(subnet)”。而在这里，我们对子网同样使用“网络”这个术语。

在网络间寻找路径有多种方法。最常用的分类方法是按照网关间交换信息的不同类型。距离向量算法仅交换少量信息，每个参与路由协议的实体(网关或主机)都保留所有目标的信息。通常，同一个网络中所有实体的信息被汇总成一个路由项，用以表示到达该网络上所有目标的路径。这是因为对IP而言，一个网络内的路由是不存在的。路由数据中的每一个项都包含了到达目标的下一跳网关，以及衡量到达目标距离的“尺度(metric)”。距离没有明确的概念，可以是到达目标的延迟、或发送信息的货币成本等等。距离向量算法的得名来源于比较不同的距离来得到最佳路径。此外，信息只在邻接，即共享同一网络的实体间交换。

虽然路由通常是基于网络信息，有时也需要保持到达独立主机的路径信息。RIP协议对待网络与主机没有差别。不管是网络还是主机，RIP都交换其信息(但是，有一些实现不支持主机路由，见3.2节)。事实上，在数学模型中将其做转换是很方便的。当在抽象描述算法的时候，最好将到达一个网络的路由项看作为所有连接该网络的实体项的缩写。这是因为在IP层，一个网络内部没有层次结构。我们也将一个给定网络中的所有项赋予同样的距离。

上面曾说到每个实体都要为每个路由项保留所有的信息。在实际中一般需要保留以下信息：

\- 地址：该算法的IP实现中，必须有主机或网络的IP地址。

\- 网关：到达目标的路径上的第一个网关。

\- 接口：到达第一个网关的物理网段。

\- 尺度：表示到达目标距离的数值。

\- 时间：表示自从该项上次更新以来的时间。

此外还包括多种标志和内部信息。这些信息在初始时包含了直接相连的实体。当从邻居网关接收到信息后，信息将会被更新。

主机和网关之间的信息主要通过更新来传递。每个参与路由协议的实体都将自己当前的路由信息作为更新信息发送。这样仅通过与邻居实体交换信息，就可以得到整个系统的最佳路径。算法将在下一节中描述。

如上所述，路由的任务是寻找到达最终目标的路径。距离向量算法是基于一张给出系统内各目标最佳路径的表。为了定义什么样的路径是最好的，需要对路径进行衡量。这就是“尺度(metric)”。

在简单系统中，常常使用的尺度是计算信息需要经过多少个网关。复杂些的系统中，将信息传输所需的延迟、所需的成本等作为尺度。还需要将各个跳数的“成本”相加。

如果实体i、j直接相连而不通过其他网关，公式d(i,j)和i、j间的费用相关。在正常情况下，给定网络上的所有实体被平等看待，其d(i,j)表示使用该网络的成本，且其值相同。衡量一条完整的路径，要将各跳的成本相加。在本备忘录中，设定成本为正整数。

公式D(i,j)表明实体i、j间的最佳尺度。这在每一对实体间都需定义。d(i,j)表示的是独立的一步。当实体i、j直接相连时，d(i,j)表示其连接成本；对实体i、j不直接相连的情况d(i,j)为无穷大。(注意d(i,i)为无穷大，因为不认为一个节点直接连接到自己。)因为费用被考虑，下面列出的是最佳尺度的计算公式。

​        D(i,i) = 0,              所有的i
​        D(i,j) = min [d(i,k) + D(k,j)], 不同的k

最佳的路径是使d(i,k) + D(k,j)达到最小的邻居k。（这样的计算在路由器上会作多次）第二个等式中的k可以被限定为i的直接邻居，不然d(i,k)将是无穷大，总的等式不可能最小。

计算就是基于这样简单的算法。实体i从其邻居k发送的信息中得到目标j的距离。当收到后，i加上i、k之间的费用d(i,k)。并比较所有的邻居取得最小值。

在[2]中证明了，当拓扑结构不变时，算法可以在有限的时间内使D(i,j)正确汇聚。作者没有假设各个实体发送信息的次序，也没有指出何时重新计算最小值。基本上，实体将不断的发送信息并重计算尺度，而且网络不能延迟这些信息(实体的崩溃将导致拓扑改变)。他们同样证明，除了可以肯定是非负数，不要对D(i,j)的初始值作出假设。事实上，不需要假设是很好也很重要的。不对实体发送信息的次序作出假设，可以使各实体按照其各自的时钟异步的运行算法；更新信息也可以被网络丢弃，只要不被全部丢弃。不对初始值作出假设，可以使算法处理拓扑改变。当系统改变后，路由算法可以从旧的状态向新的状态稳定。不然，可能会有导致不汇聚的情况。

上面的算法(以及说明)都假设每一个实体都保持了从其邻居获得信息的所有备份，并在其中得出一个最小值。实际的实现不需这样，仅仅记住一个最佳尺度，及发送这个尺度的邻居。当发现一个更好(更小)的尺度后再替换。这样可以不用存储所有邻居的信息而计算出最小值。

上面的描述和实际的RIP协议还有一个不同：在描述中，每个实体有其自身的一项，其距离为0。而事实上一般不这样。

回忆上面所说，同一个网络中所有实体的信息被汇总成一个路由项。考虑以下情况：主机或网关G连接到网络A上。C为使用网络A的成本(通常是1)。(再回忆：对IP而言，一个网络内的路由是不存在的，一个网络中的两个实体有同样的成本）在理论上，G可以得到网络A上所有实体H的信息，并认为到自身的成本为0。G将计算出到H的成本为C+0，G将仅维持一条到网络A、成本是C的项，而不是为每个H建立一项。这一项可以被认为是到达网络A上所有实体的汇总；唯一不能被汇总的项是到G自身，其成本是0而不是C。但我们从不使用0实体，可以将其和网络A的项合并。考虑这一策略的另一含义：因为我们不使用0实体，不作为网关的主机就不需要发送更新信息。不作为网关的主机(也就是只连接到一个网络的主机)，只会发送自身的一项D(i,i)=0。因为这些主机只有一个接口，要到达其他网络的路径如果到达该端口也会被从同一端口返回。这样其成本就会比C大。所以非网关根本不需要参与路由协议。

让我们来汇总一下主机或网关G的工作：对于系统中的每一个目标，G都保持当前的尺度，和发送这一尺度的邻居网关。如果目标在与G直接相连的网络上，G将使用表示网络成本的一项，而不使用任何网关。显然，当计算汇聚到正确的尺度后，按这种技术记录下的邻居将是到达目标的路径上的第一个网关(如果有多条等价路径，第一个网关将是其中之一)。也就是说将通过网关可以用多少成本到达目标。

仅当有更小的尺度到达的时候，当前保留的尺度才能被降低。尺度也可能被增加，因为一开始的估计较小。可以做这样的规定：设当前到目标的路径是使用网关G，尺度D；对于从其他网关得到的信息，只有当其尺度比D好时才使用；而对于从G来的信息，不管好坏都将被使用。可以看到，通过这样的规定得到的路由表与保持所有从邻居得到的信息得到的路由表是一致的。(注意在这里的讨论中，网络被认为是静态的，系统中没有网络会失效。)

以上是距离向量算法的简单概述。(这还不是RIP协议的描述，仍需要更精确的描述)。每一个参与路由协议的实体都需要按照下面的步骤。每个网关都必须包括，非网关主机也可以参与。

\- 保持到达系统中每个目标的表项。每一项都包括到达目标的距离和路径上的第一个网关。理论上讲，需要有指向自身，尺度为0的一项。但事实上可以不包括。

\- 向每一个邻居周期性的发送路由更新。更新中的信息包括路由表中的所有信息。包含到达每个目标的项，及到达目标的距离。

\- 邻居G'发送路由更新，将其尺度加上G'相关联的网络成本(是更新信息到达的网络)得到新的距离D'。将结果与当前路由表中的项比较。如果D'比现有的D小时，采用新的路径；即将新的路径改为使用G'，距离D'。当G'=G时，使用新的尺度，即使尺度变大。

2.1. 拓扑结构改变时的处理

以上的讨论都假设网络拓扑是固定的。实际上，网关和线路经常会出错和恢复。我们需要对算法作出一些细小的变动来处理这些情况。在理论化的算法中包括了所有的直接邻居，如果拓扑改变，在下一次计算中就会有反映。而在实际算法中，简化了算法，仅保持给定目标的最佳路径。当包含在路径中的网关或网络崩溃后，计算将不会反映这一变化。算法依赖于其邻居发现尺度改变，如果网关崩溃，就不会向其邻居报告变化。

为了解决这类问题，距离向量协议必须使用路径时效来进行预防。不同的协议，其细节不同。例如，RIP网关每30秒周期性的向其所有的邻居发送更新信息。如使用网关G到达网络N，如果有180秒没有收到G的更新，就假设网关G或连接G的网络崩溃了。这样我们就可以标志路径无效。如果从另一邻居得到到达网络N的有效路径，就将其替代无效的项。注意因为信息在传输中可能被丢失，所以将等待180秒即使每30秒就将会有更新信息。仅因为一次信息的遗失而使路径失效是不适合的。

正如我们下面将说的，将无效的路径通知其邻居是非常有用的。RIP和许多其他同类协议一样，通过在普通更新信息中标识网络为不可到达来实现。一个比最大的有效尺度更大的数值用来表示不可到达，在当前的RIP中使用16。16虽然是一个非常小的数字，但在这里通常被称为“无穷大”，很多实现都是这样规定的。其原因下面很快就要讲到。