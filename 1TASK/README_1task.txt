УСЛОВИЕ ЗАДАЧИ:
Написать две программы, не связанные отношением родитель – ребёнок,
запускающиеся в разных терминалах в разное время в любом порядке.

Одна программа открывает, читает файл и передаёт её второй программе, которая выводит данные на экран.

Чтецы и писцы могут запускаться в произвольном количестве. Файл при передаче не должен портиться. Передачу прерывать нельзя. Файловые локи, семафоры и проч., протоколы передачи данных использовать нельзя!

ВОПРОСЫ:
    ВОЗМОЖНЫЕ:
    {
        а)Критические секции
        {
            WAITING FOR ...
        }  

        б)Написать функцию, которая проверяет, соответствуют ли fd1 и fd2 одному файлу.
            /* Returns 1 if same, 0 if not and -1 in case of error.
            * Errno is set by last fstat() call.
            */

            int is_same_file(int fd1, int fd2) {
	    struct stat st1, st2;
	    if ((fstat(fd1, &st1) == -1) ||
                (fstat(fd2, &st2) == -1))
		return -1;
            else
		return (st1.st_ino == st2.st_ino) &&
                    (st1.st_dev == st2.st_dev);
    

        в)Соответствуют ли fd1 и fd2 одному файлу?
            Не всегда.

            main()
            {
	        fd1 = open("file", O_RDWR);
	        fd2 = open("file", O_RDWR);	
            }

        г)Что выведет следующая программа?
            mkfifo("fifo", 666);
            fd = open("fifo", O_RDWR);
            write(fd, "aaa", 3);
            close(fd);
            fd = open("fifo", O_RDONLY);
            write(1, buf, read(fd, buf, 3));

            Ничего:

                /* Permissions?
                 RWXRWXRWX
                *	666	1010011010
                *     0666	 110110110
                */

                /* 1) User */
                mkfifo("fifo", 666);			// ok?
                fd = open("fifo", O_RDWR);		// fd = -1
                write(fd, "aaa", 3);			// nope
                close(fd);				// nope
                fd = open("fifo", O_RDONLY);		// fd = -1
                write(1, buf, read(fd, buf, 3));	// nope

                /* 2) SU */
                mkfifo("fifo", 666);			// ok?
                fd = open("fifo", O_RDWR);		// ok
                write(fd, "aaa", 3);			// ok
                close(fd);				// ok
                fd = open("fifo", O_RDONLY);		// block
                write(1, buf, read(fd, buf, 3));	// ...
    ТЕКУЩИЕ:














