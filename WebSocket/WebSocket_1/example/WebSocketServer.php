<?php
require_once "../vendor/autoload.php";

use WebSocket\Server;

$ws = new Server("127.0.0.1", "8080");
