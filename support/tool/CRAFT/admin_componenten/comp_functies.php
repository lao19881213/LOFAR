<?php

	session_start();
?>

function switchMelding() 
{
	var y=document.getElementById('type_melding').value;
	document.getElementById('frame_melding').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_melding.php?c=" + y;
}


function SubmitComponentBewerken() 
{
	var s = document.frames['frame_melding'].document.getElementById('sStatus').value;
	var m = document.frames['frame_melding'].document.getElementById('sMelding').value;
	
	document.getElementById('hidden_melding').value = m;
	document.getElementById('hidden_status').value = s;
	
	document.theForm.submit();
}

function switchDocument(naam)
{
	var y=document.getElementById('comp_type').value;
	document.getElementById('frame_parent').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_parent.php?c=" + y + naam;
	document.getElementById('frame_naam').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_naam.php?c=" + y + naam;	
	document.getElementById('frame_fabricant').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_fabricant.php?c=" + y + naam;	
	document.getElementById('frame_leverancier').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_leverancier.php?c=" + y + naam;	
}

function submitComponentToevoegen() 
{
	var s = document.frames['frame_melding'].document.getElementById('sStatus').value;
	var m = document.frames['frame_melding'].document.getElementById('sMelding').value;
	var w = document.frames['frame_parent'].document.getElementById('sComp_Parent').value;
	var x = document.frames['frame_naam'].document.getElementById('sComp_Naam').value;
	var y = document.frames['frame_fabricant'].document.getElementById('sComp_Fabricant').value;
	var z = document.frames['frame_leverancier'].document.getElementById('sComp_Leverancier').value;
	
	var aantal  = document.frames['frame_naam'].document.getElementById('sComp_Aantal').value;
	var maximum = document.frames['frame_naam'].document.getElementById('sComp_Max').value;
	document.getElementById('hidden_type').value = w;
	document.getElementById('hidden_naam').value = x;
	document.getElementById('hidden_fabricant').value = y;
	document.getElementById('hidden_leverancier').value = z;
	document.getElementById('hidden_melding').value = m;
	document.getElementById('hidden_status').value = s;

	document.getElementById('hidden_aantal').value = aantal;
	document.getElementById('hidden_maximum').value = maximum;
	document.theForm.submit();
}