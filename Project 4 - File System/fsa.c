#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2fs.h>


int main(int argc, char *argv[]){

  int i, rv, fd;
  unsigned char byte;
  char *sb1 = NULL;
  struct ext2_super_block *sb2 = NULL;
  struct ext2_group_desc *gdesc = NULL;
  struct ext2_dir_entry_2 *root = NULL;
  struct ext2_inode *rootnode = NULL;


  if (argc != 2) {
    fprintf (stderr, "%s: Usage: ./fsa disk_image_file\n", "fsa");
    exit(1);
  }
  
  fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("disk_image_file open failed");
    exit(1);
  }
  
  /*skip the boot info - the first 1024 bytes - using the lseek*/
  if (lseek(fd, 1024, SEEK_CUR) != 1024) {
    perror("File seek failed");
    exit(1);
  }
  
  sb1 = malloc(1024);
  sb2 = malloc(sizeof(struct ext2_super_block));
  gdesc = malloc(sizeof(struct ext2_group_desc));
  root = malloc(sizeof(struct ext2_dir_entry_2));
  rootnode = malloc(sizeof(struct ext2_inode));

  if (sb1 == NULL || sb2 == NULL) {
    fprintf (stderr, "%s: Error in malloc\n", "sb");
    exit(1);
  }
  
  /*read the superblock byte by byte, print each byte, and store in sb1*/  
  for (i = 0; i < 1024; i++) {
    rv = read(fd, &byte, 1);
    if (rv == -1) {
      perror("File read failed");
      exit(1);
    }
    if (rv == 1) {
      //printf("byte[%d]: 0x%02X\n", i+1024, byte);
      sb1[i] = byte;
    }    
  }

  //printf ("Total Number of Inodes: %u\n", *(unsigned int *)sb1);
  //printf ("Number of Free Inodes: %u\n", *(unsigned int *)(sb1+16));
  

  /*set the file offset to byte 1024 again*/
  if (lseek(fd, 1024, SEEK_SET) != 1024) {
    perror("File seek failed");
    exit(1);
  }
  
  /*read the whole superblock and load into the ext2_super_block struct*/
  /*assumes the struct fields are laid out in the order they are defined*/
  rv = read(fd, sb2, sizeof(struct ext2_super_block));
  if (rv == -1) {
    perror("File read failed");
    exit(1);
  }
  if (rv == 1024) {
    //printf ("Total Number of Inodes: %u\n", sb2->s_inodes_count);
    //printf ("Number of Free Inodes: %u\n", sb2->s_free_inodes_count);
  }    
  long block_size;
  block_size = 1024 << sb2->s_log_block_size; //block size
  long disk_size;
  disk_size=(block_size)*(sb2->s_blocks_count);
  long temp = (sb2->s_inode_size)*(sb2->s_inodes_per_group);
  long iBlocks_per_g = (temp)/block_size;
  if(temp%block_size != 0){
  	iBlocks_per_g=iBlocks_per_g+1;
  }
  long group_tot =(sb2->s_blocks_count)/(sb2->s_blocks_per_group);
  if((sb2->s_blocks_count)%(sb2->s_blocks_per_group) != 0){
  	group_tot=group_tot+1;
  }

  printf("--General File System Information--\n");
  printf("Block Size in Bytes: %ld\n",block_size);
  printf("Total Number of Blocks: %d\n",sb2->s_blocks_count);
  printf("Disk Size in Bytes: %ld\n",disk_size);
  printf("Maximum Number of Blocks Per Group: %d\n",sb2->s_blocks_per_group);
  printf("Inode Size in Bytes: %d\n",sb2->s_inode_size);
  printf("Number of Inodes Per Group: %d\n",sb2->s_inodes_per_group);
  printf("Number of Inode Blocks Per Group: %ld\n",iBlocks_per_g);
  printf("Number of Groups: %ld\n",group_tot);	//why is the maximum per group greater than the total?
  printf("\n--Individual Group Information--\n");

  long g_offset = (2048/block_size);
  if(2048%block_size != 0){
  	g_offset=g_offset+1;
  }

  long go = g_offset*block_size;



  if (lseek(fd, go, SEEK_SET) != go) {
  	perror("File seek failed");
  	exit(1);
  }
  unsigned char bitm[block_size];
  long fblock = sb2->s_first_data_block;
  for(i=0;i<group_tot;i++){
  	rv = read(fd, gdesc, sizeof(struct ext2_group_desc));
  	if (rv == -1) {
    	perror("File read failed");
    	exit(1);
  	}

    long eblock;
  	if(i+1==group_tot){
  		long t1 = (group_tot-1)*(sb2->s_blocks_per_group);
  		g_offset = (sb2->s_blocks_count)-t1;
      eblock=fblock+g_offset-1;
  	}
  	else{
  		g_offset=g_offset+sb2->s_blocks_per_group;
      eblock=fblock+(sb2->s_blocks_per_group-1);
  	}
  	printf("-Group %d-\n",i);
  	printf("Block IDs: %ld-%ld\n",fblock,eblock);
  	printf("Block Bitmap Block ID: %d\n",gdesc->bg_block_bitmap);
  	printf("Inode Bitmap Block ID: %d\n",gdesc->bg_inode_bitmap);
  	printf("Inode Table Block ID: %d\n",gdesc->bg_inode_table);
  	printf("Number of Free Blocks: %d\n",gdesc->bg_free_blocks_count);
  	printf("Number of Free Inodes: %d\n",gdesc->bg_free_inodes_count);
  	printf("Number of Directories: %d\n",gdesc->bg_used_dirs_count);

    long idtot= eblock-fblock+1;
    //printf("%ld",idtot);
    long currbl = fblock;
    //printf("%ld",currbl);
    //long blocks_ingroup = g_offset-fblock;
    long bm_offset = (block_size*gdesc->bg_block_bitmap)-1;
    //printf("%ld",bm_offset);
    if (lseek(fd,bm_offset, SEEK_SET) != bm_offset){
      printf("byte[%d]: 0x%02X\n", i, byte);
      sb1[i] =byte;
    }
    rv = read(fd, bitm, sizeof(bitm));
    if (rv == -1) {
      perror("File read failed");
      exit(1);
    }
    int check[8];
    //char temp;
    long unusedbl[sb2->s_blocks_per_group];
    for(i=0;i<(idtot/8);i++){ //8 blocks are accounted for per byte
      //strcpy(temp,bitm[i]);
      for (int j = 0; j < 8; ++j) {
        check[j]= (( bitm[i] & (1 << i) ) ? 1 : 0);
        if(check[j] != 0){
          check[j]=1;
        }
        if(check[j] == 0){ //each time there is a free 
          unusedbl[(j+(8*i))]=currbl;
          //printf("%ld\n",currbl);
        }
        currbl++; //keeps track of current block, used to keep data on what blocks are open
      //printf("%d\n",b[i]);
      }
      fblock=eblock+1;
    }
    //printf("%ld",currbl);
    /*char a = 'a';
    int b[8];
    for (int i = 0; i < 8; ++i) {
      b[i]= (( a & (1 << i) ) ? 1 : 0);
      if(b[i] != 0){
        b[i]=1;
      }
      printf("%d\n",b[i]);
    }*/
    printf("Free Block IDs: ");
    long flag =1;
    printf("%ld",unusedbl[0]);
    while(flag !=0){
      for(i=1;i<idtot;i++){
        flag=unusedbl[i];
        if(flag == 0){
          break;
        }
        if(unusedbl[i]==(unusedbl[i-1]+1)){

        }
        else{
          if(unusedbl[i-1]==(unusedbl[i-3]+2)){
            printf("-%ld,%ld",unusedbl[i-1],unusedbl[i]);
          }
          else{
            if(i != 1){
              printf(",%ld,%ld",unusedbl[i-1],unusedbl[i]);
            }
            else{
              printf(",%ld",unusedbl[i]);
            }
          }
        }
      }
    }

long idnodetot= sb2->s_inodes_per_group;
    //printf("%ld",idtot);
    long currnodebl = 1;
    //printf("%ld",currbl);
    //long blocks_ingroup = g_offset-fblock;
    long bmnode_offset = (block_size*gdesc->bg_inode_bitmap)-1;
    //printf("%ld",bm_offset);
    if (lseek(fd,bmnode_offset, SEEK_SET) != bmnode_offset){
      printf("byte[%d]: 0x%02X\n", i, byte);
      sb1[i] =byte;
    }
    rv = read(fd, bitm, sizeof(bitm));
    if (rv == -1) {
      perror("File read failed");
      exit(1);
    }
    //int check[8];
    //char temp;
    long unusednodebl[idnodetot];
    for(i=0;i<((idnodetot/8));i++){ //8 blocks are accounted for per byte
      //strcpy(temp,bitm[i]);
      for (int j = 0; j < 8; ++j) {
        check[j]= (( bitm[i] & (1 << i) ) ? 1 : 0);
        if(check[j] != 0){
          check[j]=1;
        }
        if(check[j] == 0){ //each time there is a free 
          unusednodebl[(j+(8*i))]=currbl;
          //printf("%ld\n",currbl);
        }
        currnodebl++; //keeps track of current block, used to keep data on what blocks are open
      //printf("%d\n",b[i]);
      }

    }
    //printf("%ld",currbl);
    /*char a = 'a';
    int b[8];
    for (int i = 0; i < 8; ++i) {
      b[i]= (( a & (1 << i) ) ? 1 : 0);
      if(b[i] != 0){
        b[i]=1;
      }
      printf("%d\n",b[i]);
    }*/
    printf("\nFree Inode IDs: ");
    flag =1;
    printf("%ld",unusednodebl[0]);
    while(flag !=0){
      for(i=1;i<idnodetot;i++){
        flag=unusednodebl[i];
        if(flag == 0){
          break;
        }
        if(unusednodebl[i]==(unusednodebl[i-1]+1)){

        }
        else{
          if(unusednodebl[i-1]==(unusednodebl[i-3]+2)){
            printf("-%ld,%ld",unusednodebl[i-1],unusednodebl[i]);
          }
          else{
            if(i != 1){
              printf(",%ld,%ld",unusednodebl[i-1],unusednodebl[i]);
            }
            else{
              printf(",%ld",unusednodebl[i]);
            }
          }
        }
      }
    }
  }

  g_offset = (2048/block_size);
  if(2048%block_size != 0){
    g_offset=g_offset+1;
  }

  go = g_offset*block_size;

  if (lseek(fd, go, SEEK_SET) != go) {
    perror("File seek failed");
    exit(1);
  }

  rv = read(fd, gdesc, sizeof(struct ext2_group_desc));
  if (rv == -1) {
    perror("File read failed");
    exit(1);
  }
  long rootoffset = (block_size*gdesc->bg_inode_table)+128; //skips to the inode table and then to the second inode
  //printf("%ld\n",rootoffset);
  lseek(fd,rootoffset,SEEK_SET);
  rv = read(fd, rootnode, sizeof(struct ext2_inode));
  if (rv == -1) {
    perror("File read failed");
    exit(1);
  }

  printf("\n\n--Root Directory Entries--\n");
  //printf("%d\n",rootnode->i_block[1]);
  long tot_length = 0;
  //int count = 0;
 /* while(tot_length != block_size){
    for(i=0;i<12;i++){*/
      long ioff = (block_size*rootnode->i_block[i]);
      lseek(fd,ioff,SEEK_SET);
      rv = read(fd, root, sizeof(root));
      if (rv == -1) {
        perror("File read failed");
        exit(1);
      }
      printf("Inode: %d\n",root->inode);
      printf("Entry Length: %d\n",root->rec_len);
      printf("Name Length: %d\n",root->name_len);
      printf("File Type: %d\n",root->file_type);
      printf("Name: ");
      for(i=0;i<root->name_len;i++){
        printf("%c",root->name[i]);
      }
      printf("\n");
      tot_length = tot_length+root->rec_len;
    /*}
    break;
  }*/

  free(sb1);
  free(sb2);
  free(gdesc);
  free(root);
  free(rootnode);
  close(fd);
  
  return 0;
}