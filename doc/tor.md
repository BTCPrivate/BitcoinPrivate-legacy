*** Warning: Do not assume Tor support does the correct thing in Zcash; better Tor support is a future feature goal. ***

TOR SUPPORT IN ZCASH
====================

It is possible to run Zcash as a Tor hidden service, and connect to such services.

The following directions assume you have a Tor proxy running on port 9050. Many distributions default to having a SOCKS proxy listening on port 9050, but others may not. In particular, the Tor Browser Bundle defaults to listening on port 9150. See [Tor Project FAQ:TBBSocksPort](https://www.torproject.org/docs/faq.html.en#TBBSocksPort) for how to properly
configure Tor.


1. Run Zcash behind a Tor proxy
-------------------------------

The first step is running Zcash behind a Tor proxy. This will already make all
outgoing connections be anonymized, but more is possible.

	-proxy=ip:port  Set the proxy server. If SOCKS5 is selected (default), this proxy
	                server will be used to try to reach .onion addresses as well.

	-onion=ip:port  Set the proxy server to use for Tor hidden services. You do not
	                need to set this if it's the same as -proxy. You can use -noonion
	                to explicitly disable access to hidden service.

	-listen         When using -proxy, listening is disabled by default. If you want
	                to run a hidden service (see next section), you'll need to enable
	                it explicitly.

	-connect=X      When behind a Tor proxy, you can specify .onion addresses instead
	-addnode=X      of IP addresses or hostnames in these parameters. It requires
	-seednode=X     SOCKS5. In Tor mode, such addresses can also be exchanged with
	                other P2P nodes.

In a typical situation, this suffices to run behind a Tor proxy:

	./btcpd -proxy=127.0.0.1:9050


2. Run a Zcash hidden server
----------------------------

If you configure your Tor system accordingly, it is possible to make your node also
reachable from the Tor network. Add these lines to your /etc/tor/torrc (or equivalent
config file):

	HiddenServiceDir /var/lib/tor/zcash-service/
	HiddenServicePort 7933 127.0.0.1:7933
	HiddenServicePort 17933 127.0.0.1:17933

The directory can be different of course, but (both) port numbers should be equal to
your btcpd's P2P listen port (7933 by default).

	-externalip=X   You can tell Zcash about its publicly reachable address using
	                this option, and this can be a .onion address. Given the above
	                configuration, you can find your onion address in
	                /var/lib/tor/zcash-service/hostname. Onion addresses are given
	                preference for your node to advertize itself with, for connections
	                coming from unroutable addresses (such as 127.0.0.1, where the
	                Tor proxy typically runs).

	-listen         You'll need to enable listening for incoming connections, as this
	                is off by default behind a proxy.

	-discover       When -externalip is specified, no attempt is made to discover local
	                IPv4 or IPv6 addresses. If you want to run a dual stack, reachable
	                from both Tor and IPv4 (or IPv6), you'll need to either pass your
	                other addresses using -externalip, or explicitly enable -discover.
	                Note that both addresses of a dual-stack system may be easily
	                linkable using traffic analysis.

In a typical situation, where you're only reachable via Tor, this should suffice:

	./btcpd -proxy=127.0.0.1:9050 -externalip=zctestseie6wxgio.onion -listen

(obviously, replace the Onion address with your own). It should be noted that you still
listen on all devices and another node could establish a clearnet connection, when knowing
your address. To mitigate this, additionally bind the address of your Tor proxy:

	./btcpd ... -bind=127.0.0.1

If you don't care too much about hiding your node, and want to be reachable on IPv4
as well, use `discover` instead:

	./btcpd ... -discover

and open port 7933 on your firewall (or use -upnp).

If you only want to use Tor to reach onion addresses, but not use it as a proxy
for normal IPv4/IPv6 communication, use:

	./btcpd -onion=127.0.0.1:9050 -externalip=zctestseie6wxgio.onion -discover


3. Automatically listen on Tor
--------------------------------

Starting with Tor version 0.2.7.1 it is possible, through Tor's control socket
API, to create and destroy 'ephemeral' hidden services programmatically.
Zcash has been updated to make use of this.

This means that if Tor is running (and proper authentication has been configured),
Zcash automatically creates a hidden service to listen on. Zcash will also use Tor
automatically to connect to other .onion nodes if the control socket can be
successfully opened. This will positively affect the number of available .onion
nodes and their usage.

This new feature is enabled by default if Zcash is listening (`-listen`), and
requires a Tor connection to work. It can be explicitly disabled with `-listenonion=0`
and, if not disabled, configured using the `-torcontrol` and `-torpassword` settings.
To show verbose debugging information, pass `-debug=tor`.

Connecting to Tor's control socket API requires one of two authentication methods to be
configured. For cookie authentication the user running btcpd must have write access
to the `CookieAuthFile` specified in Tor configuration. In some cases this is
preconfigured and the creation of a hidden service is automatic. If permission problems
are seen with `-debug=tor` they can be resolved by adding both the user running tor and
the user running btcpd to the same group and setting permissions appropriately. On
Debian-based systems the user running btcpd can be added to the debian-tor group,
which has the appropriate permissions. An alternative authentication method is the use
of the `-torpassword` flag and a `hash-password` which can be enabled and specified in
Tor configuration.


4. Connect to a Zcash hidden server
-----------------------------------

To test your set-up, you might want to try connecting via Tor on a different computer to just a
a single Zcash hidden server. Launch btcpd as follows:

	./btcpd -onion=127.0.0.1:9050 -connect=zctestseie6wxgio.onion

Now use btcp-cli to verify there is only a single peer connection.

	btcp-cli getpeerinfo

	[
	    {
	        "id" : 1,
	        "addr" : "zctestseie6wxgio.onion:17933",
	        ...
	        "version" : 170002,
	        "subver" : "/MagicBean:1.0.0/",
	        ...
	    }
	]

To connect to multiple Tor nodes, use:

	./btcpd -onion=127.0.0.1:9050 -addnode=zctestseie6wxgio.onion -dnsseed=0 -onlynet=onion
