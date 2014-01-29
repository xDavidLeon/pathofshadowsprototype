	clearListener()

fn testfn obj =
(
	print $.name
	
	local userProperties = filterstring (getUserPropBuffer obj) "\r\n"
	print userProperties[1]
)

--testfn $