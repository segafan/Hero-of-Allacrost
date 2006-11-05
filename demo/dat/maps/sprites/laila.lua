-- Create the laila map sprite
sp = MapSprite()
sp:SetName("Laila")
sp:SetID(1)
-- 4 is NPC_SPRITE (still needs to be bound to lua)
sp:SetObjectType(4)
sp:SetColPosition(4)
sp:SetRowPosition(4)
---- 400.0 is NORMAL_SPEED
sp:SetStepSpeed(400.0)
---- 28 = UPDATEABLE (0x0010) | VISIBLE (0x0008) | ALWAYS_IN_CONTEXT (0x0004)
sp:SetStatus(28)
sp:SetFilename("img/sprites/map/laila")
sp:SetPortrait("img/portraits/map/laila.png")
---- 2 = SOUTH
sp:SetDirection(2)
sp:LoadFrames()
--SetOccupied(sp:GetColPosition, sp:GetRowPosition)
--SetOccupied(4,4)

sd = SpriteDialogue()
sd:AddText("It's really dark in here isn't it? I wonder how much longer our torches will last us...")
sd:AddSpeaker(1)
sp:AddDialogue(sd)

sd = SpriteDialogue()
sd:AddText("If only we had more art, maybe the designers would have put in an exit in this cave!")
sd:AddSpeaker(1)
sd:AddText("Well, they're really under staffed in the art department. We can't blame them too much.")
sd:AddSpeaker(0)
sp:AddDialogue(sd)

sd = SpriteDialogue()
sd:AddText("Did you know that you can toggle off random encounters by pressing the swap key (default: a)?")
sd:AddSpeaker(1)
sp:AddDialogue(sd)

sa = ActionPathMove()
sa:SetDestination(4,16)
sa:SetSprite(sp)
sp:AddAction(sa)

sa = ActionPathMove()
sa:SetDestination(12,16)
sa:SetSprite(sp)
sp:AddAction(sa)

sa = ActionPathMove()
sa:SetDestination(8,4)
sa:SetSprite(sp)
sp:AddAction(sa)

sa = ActionPathMove()
sa:SetDestination(4,4)
sa:SetSprite(sp)
sp:AddAction(sa)

AddGroundObject(sp)