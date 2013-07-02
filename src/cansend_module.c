#include <can_config.h>

#include <getopt.h> //getopt_long... parses '-' and '--' command line arguments
#include <libgen.h>
#include <signal.h>
#include <stdio.h> //perror
#include <stdlib.h> //strtoul...converts a string to a long integer
#include <string.h> //strcpy...
#include <unistd.h> 
#include <limits.h>

#include <net/if.h> //low level access to linux network devices

#include <sys/ioctl.h>//low level access to linux network devices
#include <sys/socket.h>//socket... write
#include <sys/types.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <Python.h> //Pulls in the python API

extern int optind, opterr, optopt//these are external variables that help parse the argument variables to look for option characters (flags)

// static void print_usage(char *prg)
// {
// 	fprintf(stderr,
// 		"Usage: %s [<can-interface>] [Options] <can-msg>\n"
// 		"<can-msg> can consist of up to 8 bytes given as a space separated list\n"
// 		"Options:\n"
// 		" -i, --identifier=ID	CAN Identifier (default = 1)\n"
// 		" -r  --rtr		send remote request\n"
// 		" -e  --extended	send extended frame\n"
// 		" -f, --family=FAMILY	Protocol family (default PF_CAN = %d)\n"
// 		" -t, --type=TYPE	Socket type, see man 2 socket (default SOCK_RAW = %d)\n"
// 		" -p, --protocol=PROTO	CAN protocol (default CAN_RAW = %d)\n"
// 		" -l			send message infinite times\n"
// 		"     --loop=COUNT	send message COUNT times\n"
// 		" -v, --verbose		be verbose\n"
// 		" -h, --help		this help\n"
// 		"     --version		print version information and exit\n",
// 		prg, PF_CAN, SOCK_RAW, CAN_RAW);
// }

// enum {
// 		VERSION_OPTION = CHAR_MAX + 1,
// };

static PyObject * //This needs to be put above every python function
int send(PyObject *self,pyObject *args)
{
	struct can_frame frame = { //creates a canframe somehow (structure defined in can.h) not sure what .can_id is
		.can_id = 1,
	};
	struct ifreq ifr; //contains some socket addresses...http://linux.die.net/man/7/netdevice
	struct sockaddr_can addr; //contains socket addresses, lives in aunion inside ifreq...http://linux.die.net/man/7/netdevice
	char *interface; //create an interface string
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW; //define the family, type, and proto for the CAN protocol
	int loopcount = 1, infinite = 0; //set a loop count and inifnite integer
	int s, opt, ret, i, dlc = 0, rtr = 0, extended = 0; //set a bunch more integers. dlc is a loop counter for populating the can frame
	int verbose = 0;

	// struct option long_options[] = {
	// 	{ "help",	no_argument,		0, 'h' },
	// 	{ "identifier",	required_argument,	0, 'i' },
	// 	{ "rtr",	no_argument,		0, 'r' },
	// 	{ "extended",	no_argument,		0, 'e' },
	// 	{ "family",	required_argument,	0, 'f' },
	// 	{ "protocol",	required_argument,	0, 'p' },
	// 	{ "type",	required_argument,	0, 't' },
	// 	{ "version",	no_argument,		0, VERSION_OPTION},
	// 	{ "verbose",	no_argument,		0, 'v'},
	// 	{ "loop",	required_argument,	0, 'l'},
	// 	{ 0,		0,			0, 0 },
	// };

	while ((opt = getopt_long(argc, argv, "hf:t:p:vi:lre", long_options, NULL)) != -1) {
		switch (opt) {
	// 	case 'h':
	// 		// if an argument is -h, print the usage instructions and exit
	// 		print_usage(basename(argv[0]));
	// 		exit(0);

	// 	case 'f':
	// 		family = strtoul(optarg, NULL, 0);
	// 		break;

	// 	case 't':
	// 		type = strtoul(optarg, NULL, 0);
	// 		break;

	// 	case 'p':
	// 		proto = strtoul(optarg, NULL, 0);
	// 		break;
 // //low level access to linux network devices
	// 	case 'v':
	// 		verbose = 1;
	// 		break;//low level access to linux network devices

		// case 'l':
		// 	if (optarg)
		// 		loopcount = strtoul(optarg, NULL, 0);
		// 	else
		// 		infinite = 1;
		// 	break;
		case 'i':
			frame.can_id = strtoul("0x601" NULL, 0);
			break;

		// case 'r':
		// 	rtr = 1;
		// 	break;

		// case 'e':
		// 	extended = 1;
		// 	break;

		// case VERSION_OPTION:
		// 	printf("cansend %s\n", VERSION);
		// 	exit(0);

		// default:
		// 	fprintf(stderr, "Unknown option %c\n", opt);
		// 	break;
		}
	}
	/*
	if the option index is equal to the arg count (true if there are no flags) then
	print something and then exit
	*/
	// if (optind == argc) { 
	// 	print_usage(basename(argv[0]));//
	// 	exit(0);
	}
	/*
	If the option index is not equal to 1, check the value at the correct option index
	and see if is null. If it is, print something to standard error and exit
	*/
	// if (argv[optind] == NULL) {
	// 	fprintf(stderr, "No Interface supplied\n");
	// 	exit(-1);
	// }

	/*
	If we haven't exited by this point, it means our options are valid and the interface is our argument
	*/
	interface = argv[optind];//not sure where this is coming from... it doesn't get passed in as an argument. perhaps can0? probably because it's just a string (see printf below)

	/*
	Now that we know our interface, print some nice information regarding the interface
	*/

	// printf("interface = %s, family = %d, type = %d, proto = %d\n",
	//        interface, family, type, proto);

	s = socket(family, type, proto);//http://linux.die.net/man/2/socket
	if (s < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strcpy(ifr.ifr_name, interface); //copies interface string to  ifr.ifr_name
	 //contains some socket addressesif (ioctl(s, SIOCGIFINDEX, &ifr)) {
		perror("ioctl")
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex; //contains some socket addresses

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}


	/*
	the following loops through a range created by doing some fanciness with optind and argc and
	for each loop it converts the string to a long integer. dlc keeps track of how many values are in the frame
	*/

	for (i = optind + 1; i < argc; i++) {
		frame.data[dlc] = strtoul(argv[i], NULL, 0);
		dlc++;
		if (dlc == 8)
			break;
	}
	frame.can_dlc = dlc; //sets the count of bytes (i think) in the can frame


	if (extended) {//it won't be extended initially
		frame.can_id &= CAN_EFF_MASK;//variablles set in can.h
		frame.can_id |= CAN_EFF_FLAG;//variablles set in can.h
	} else {
		frame.can_id &= CAN_SFF_MASK;//variablles set in can.h
	}

	if (rtr)
		frame.can_id |= CAN_RTR_FLAG; //variablles set in can.h

	if (verbose) { //there will be no need to be verbose
		printf("id: %d ", frame.can_id);
		printf("dlc: %d\n", frame.can_dlc);
		for (i = 0; i < frame.can_dlc; i++)
			printf("0x%02x ", frame.data[i]);
		printf("\n");
	}

	while (infinite || loopcount--) {
		ret = write(s, &frame, sizeof(frame));//http://linux.die.net/man/2/write
		if (ret == -1) {
			perror("write");
			break;
		}
	}

	close(s);
	return 0;
}
