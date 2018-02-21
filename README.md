# ClusterBan

ClusterBan identifies attacks and scans through a combination of command-line requests (can be called from various log scanning tools) and custom functionality similar to a "Honey-Pot".  When ClusterBan identifies a scan or an attack, it bans the source address throughout the entire cluster.

The initial bans of an IP address are temporary, but persistent attacks can cause the ban to become permanent.

Because of its design, ClusterBan has the ability to block some zero-day attacks.  In addition, it can be configured to block DDoS attacks at the Load Balancer.

Current status - Initial design

This project is licensed under the terms of the original BSD 4-clause license.  Please see the "LICENSE" file for details.
