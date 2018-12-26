#include "lookuproute.h"

std::vector<route> route_table;

/*
 * 参数 1 是目的地址,参数 2 是掩码长度,参数 3 是接口名,参数 4 是接口索引值,参数 5 是下一跳 IP 地址。
 */
int32_t insert_route(uint32_t ip4prefix, uint32_t prefixlen, char *ifname, uint32_t nexthopaddr)
{
	uint32_t ifindex = if_nametoindex(ifname);
	fprintf(stderr, "insert %d.%d.%d.%d/%d %s %d.%d.%d.%d size #%lu\n", TOIP(ip4prefix), prefixlen, ifname, TOIP(nexthopaddr), route_table.size());
	// insert 192.168.6.0/24 eth14 192.168.3.2
	route tmp;
	tmp.ip4prefix.s_addr = ntohl(ip4prefix);
	tmp.prefixlen = prefixlen;
	tmp.nexthop = new nexthop;
	tmp.nexthop->ifname = ifname;
	tmp.nexthop->ifindex = ifindex;
	tmp.nexthop->nexthopaddr.s_addr = nexthopaddr;
	// fprintf(stderr, "nextaddr: %d.%d.%d.%d %d.%d.%d.%d\n", nexthopaddr, tmp.nexthop->nexthopaddr.s_addr);
	tmp.nexthop->next = NULL;
	route_table.push_back(tmp);
	return 0;
}

int32_t modify_route(uint32_t ip4prefix, uint32_t prefixlen, char* ifname, uint32_t nexthopaddr) {
	ip4prefix = ntohl(ip4prefix);
	uint32_t ifindex = if_nametoindex(ifname);
	for (int i = 0; i < route_table.size(); ++i) {
		uint32_t tmpip = route_table[i].ip4prefix.s_addr;
		uint32_t pre = route_table[i].prefixlen;
		if (tmpip == ip4prefix && pre == prefixlen) {
			route_table[i].nexthop->ifname = ifname;
			route_table[i].nexthop->ifindex = ifindex;
			route_table[i].nexthop->nexthopaddr.s_addr = nexthopaddr;
			break;
		}
	}
}

/*
 * 参数1是目的IP地址，参数2下一跳和出接口信息
 * 注意：查找算法与所用路由表存储结构相关
 */
int32_t lookup_route(uint32_t dstaddr, nexthop *nexthopinfo)
{
	fprintf(stderr, "lookup: %d.%d.%d.%d in #%lu\n", TOIP(dstaddr), route_table.size());
	for (int i = 0; i < route_table.size(); ++i) {
		printf("--- %3d.%3d.%3d.%3d/%2d %3d.%3d.%3d.%3d\n", TOIP(ntohl(route_table[i].ip4prefix.s_addr)), route_table[i].prefixlen, TOIP(route_table[i].nexthop->nexthopaddr.s_addr));
	}
	uint32_t dstip = ntohl(dstaddr);
	uint32_t mask = 0;
	uint32_t full_mask = 0xFFFFFFFF;
	for (int i = 0; i < route_table.size(); ++i) {
		uint32_t tmpip = route_table[i].ip4prefix.s_addr;
		uint32_t pre = route_table[i].prefixlen;
		// fprintf(stderr, "diff %x %x\n%x %x\n", tmpip, dstip, (tmpip & (full_mask << (32 - pre))), (dstip & (full_mask << (32 - pre))));
		if (pre >= mask && (tmpip & (full_mask << (32 - pre))) == (dstip & (full_mask << (32 - pre)))) {
			mask = pre;
			memcpy(nexthopinfo, route_table[i].nexthop, sizeof(nexthop));
			// nexthopinfo = (*i).nexthop;
			fprintf(stderr, "match %d.%d.%d.%d/%d\n", TOIP(ntohl(tmpip)), pre);
		}
	}
	if (mask == 0)
		return -1;
	else {
		fprintf(stderr, "final: %d.%d.%d.%d\n", TOIP(nexthopinfo->nexthopaddr.s_addr));
		return 0;
	}
}

/*
 * 参数1是目的ip地址，参数2是掩码长度
 */
int32_t delete_route(uint32_t dstaddr, uint32_t prefixlen)
{
	fprintf(stderr, "delete route: %d.%d.%d.%d/%d size %lu\n", TOIP(dstaddr), prefixlen, route_table.size());
	// delete route: 192.168.3.2/24
	for (std::vector<route>::iterator i = route_table.begin(); i != route_table.end();)
		if ((*i).prefixlen == prefixlen && ((*i).nexthop)->nexthopaddr.s_addr == dstaddr)
			i = route_table.erase(i);
		else
			++i;
}
