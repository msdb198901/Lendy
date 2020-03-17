-- 

START TRANSACTION; -- we're messing with the accounts table here, let's play it safe

ALTER TABLE `account_info` ADD COLUMN `logon_type` tinyint(3) NOT NULL DEFAULT '0' AFTER `service_limit`;

COMMIT; -- safety gloves off
