unsigned int hash(char *s) {//Lifted unashamedly from K&R2.
	unsigned hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31*hashval;
	return hashval%64;
}

int main(int argc, char** argv) {
	if(!argc) {
		printf("Need key.\n");
		return 1;
	}
	int targetHash=hash(argv[1]);
	int newHash=-1;
	char clash[32]={0};
	printf("Finding collision for %s, with hash %d.\n",argv[1],targetHash);
	while(targetHash!=newHash) {
		for(int i=0;i<31;i++)
		{
			clash[i]=(rand()%26)+65;
			clash[31]='\0';
		}
		newHash=hash(clash);
	}
	printf("%s has hash %d, as does %s\n", clash, newHash, argv[1]);
	return 0;
}

