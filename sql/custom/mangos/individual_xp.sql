DELETE FROM `command` WHERE `name` IN ('xp', 'xp set', 'xp current', 'xp available', 'xp default');
INSERT INTO `command` (`name`, `security`, `help`) VALUES 
('xp', 0, 'Syntax: .xp $subcommand\r\n\r\nIndividual XP rate commands. Type .help xp to see a list of subcommands\r\nor .help xp $subcommand to see info on the subcommand.'),
('xp set', 0, 'Syntax: .xp set $rate\r\n\r\nSet your custom XP rate. Rate must be between 0.1 and the server maximum.\r\nExample: .xp set 2.0'),
('xp current', 0, 'Syntax: .xp current\r\n\r\nView your current XP rate.'),
('xp available', 0, 'Syntax: .xp available\r\n\r\nShow the range of XP rates you can set.'),
('xp default', 0, 'Syntax: .xp default\r\n\r\nReset your XP rate to the default value.');

DELETE FROM `mangos_string` WHERE `entry` IN (11000, 11001, 11002, 11003, 11004, 11005, 11006, 11007, 11008, 11009);
INSERT INTO `mangos_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES
(11000, 'Usage: .xp set <rate>', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11001, 'Example: .xp set 5.0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11002, 'You can set your XP rate between 0.1 and %.1f', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11003, 'XP rate too low. Minimum rate is 0.1', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11004, 'XP rate too high. Maximum rate is %.1f', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11005, 'You have set your XP rate to %.1f', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11006, 'Current XP rate: %.1f', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11007, 'Your XP rate has been reset to the default value.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11008, 'Current XP rate: %.1f', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
(11009, 'Your XP rate is: %.1f (Max: %u)', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);