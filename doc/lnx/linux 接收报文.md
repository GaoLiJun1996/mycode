# linux 接收报文

Linux 接收报文的过程从硬件层面的网卡驱动开始，经过软中断处理和协议栈的多层处理，最终将数据包传递给应用程序。每个环节都有相应的函数和数据结构负责处理和管理数据包的流动，保证了网络通信的有效性和性能。

## 函数角度

| 过程说明               |                                                              |                                                      |
| ---------------------- | ------------------------------------------------------------ | ---------------------------------------------------- |
| 网卡接收数据包         | 硬件接收数据包交由驱动程序处理                               |                                                      |
| 网卡驱动接收数据包处理 | netif_rx  ——>__skb_queue_tail/netif_rx_schedule              | netif_rx 处理两件事情：将skb添加到队列；进行接收调度 |
| 软中断处理             | __netif_rx_schedule——>__raise_softirq_irqoff(NET_RX_SOFTIRQ);——>net_rx_action->dev->poll() ——>process_backlog()——>netif_receive_skb()->deliver_skb() | deliver_skb 根据skb 中相应属性 进行处理              |
| sk_buff 结构体处理     | sk_buff                                                      |                                                      |
| 协议栈处理             | vlan_packet_type——vlan处理函数**vlan_skb_recv**              |                                                      |

### 流程图

![](https://yian-1324200595.cos.ap-guangzhou.myqcloud.com/imgLinux%E6%8E%A5%E5%8F%97%E6%8A%A5%E6%96%87.drawio.svg)

## 数据角度

## 中断机制