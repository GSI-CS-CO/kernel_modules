#
# icv196.awk - extract insmod parameters from transfer.ref
#
# usage: icv196.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f icv196.awk ICV196 /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#     insmod icv196.ko dname=icv196 luns=1,2,3
#                                   vme1=0x980000,0xa00000,0xa80000
#                                   vme2=0x980000,0xa00000,0xa80000
#                                   vecs=0xB0,0xB1,0xB2
#

BEGIN	{
	device_name = ARGV[1]
	delete ARGV[1]
	luns = ""
	base_address1 = ""
	base_address2 = ""
	vectors = ""
}

/^#\+#/ && $6 == device_name  && $4 == "VME" {
	# decode transfer.ref line
	luns =  luns "," $7
	base_address1 =  base_address1 "," "0x" $11
	base_address2 =  base_address2 "," "0x" $16
	vectors =  vectors "," $23
}

END {

	insmod_params = " "

	if (luns) insmod_params = insmod_params "luns=" substr(luns, 2)

	if (vectors) insmod_params = insmod_params " vecs=" substr(vectors, 2)

	if (base_address1) insmod_params = insmod_params " vmeb=" substr(base_address1, 2)

	print insmod_params
}
