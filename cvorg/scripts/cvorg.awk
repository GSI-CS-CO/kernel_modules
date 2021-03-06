#
# cvorg.awk - extract insmod parameters from transfer.ref
#
# usage: transfer2insmod.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f cvorg.awk CVORG /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#     insmod cvorg.ko luns=1,2,3
#                     vme1=0x980000,0xa00000,0xa80000
#                     vme2=0x980000,0xa00000,0xa80000
#                     vecs=0xB0,0xB1,0xB2
#

BEGIN	{
	device_name = ARGV[1]
	delete ARGV[1]
	luns = ""
	base_address1 = ""
	vectors = ""
}

/^#\+#/ && $6 == device_name  && $4 == "VME" {
	# decode transfer.ref line
	luns =  luns "," $7
	base_address1 =  base_address1 "," "0x" $11
	vectors =  vectors "," $23
}

END {

	insmod_params = " "

	if (luns) insmod_params = insmod_params "lun=" substr(luns, 2)

	if (vectors) insmod_params = insmod_params " irq=" substr(vectors, 2)

	if (base_address1) insmod_params = insmod_params " base_address=" substr(base_address1, 2)

	print insmod_params
}
