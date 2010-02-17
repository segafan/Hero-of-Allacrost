-- Add a new language by creating a new entry in the languages table.
-- The first entry after the open-parenthesis is the name of the language
-- as it will appear in the Language Selection menu in the game.
-- The second entry corresponds to the name of the gettext PO file for that
-- language.
languages = {}
languages[1] = { hoa_system.Translate("English"), "en@quot" }
languages[2] = { hoa_system.Translate("French"), "fr" }
