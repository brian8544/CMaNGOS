DROP TABLE IF EXISTS `individualxp`;
CREATE TABLE `individualxp` (
  `CharacterGUID` INT UNSIGNED NOT NULL,
  `XPRate` FLOAT NOT NULL DEFAULT 1,
  PRIMARY KEY (`CharacterGUID`),
  CONSTRAINT `individualxp_chk_1` CHECK (`XPRate` >= 0)
)