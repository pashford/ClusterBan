# ClusterBan

ClusterBan identifies attacks and scans through a combination of log scanning, using the "Fail2Ban" Open-Source Software, and custom functionality similar to a "Honey-Pot".

Most of the blocking is temporary, but it can be set to permanently block persistent threats.

When threats are identified, the source IP address is banned in the Service Cluster.         

Because of its design, ClusterBan has the ability to block some zero-day attacks.

Current status - Initial design
