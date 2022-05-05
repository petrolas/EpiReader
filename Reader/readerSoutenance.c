#include <stdio.h>
#include <stdlib.h>
#include <err.h>

void addMask111(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if(j%3 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask110(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if((i+j)%3 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask101(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if((i+j)%2 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask100(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if(i%2 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask011(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if((((i*j)%3)+i*j)%2 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask010(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if(((i*j)%3+i+j)%2 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask001(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if((i/2 + j/3)%2 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask000(char qrcode[20][20])
{
    for(size_t i = 1; i<=20; i++)
    {
        for(size_t j = 1; j<=20; j++)
        {
            if((i*j)%2+(i*j)%3 == 0)
                qrcode[i-1][j-1] =  (qrcode[i-1][j-1]+1)%2;
        }
    }
}
void addMask(int mask, char qrcode[20][20])
{
    switch (mask)
    {
    case 0:
        addMask000(qrcode);
        break;
    case 1:
        addMask100(qrcode);
        break;
    case 2:
        addMask010(qrcode);
        break;
    case 3:
        addMask110(qrcode);
        break;
    case 4:
        addMask001(qrcode);
        break;
    case 5:
        addMask101(qrcode);
        break;
    case 6:
        addMask011(qrcode);
        break;
    case 7:
        addMask111(qrcode);
        break;
    default:
        break;
    }
}
int isValid(size_t x, size_t y)
{
    if(x<=7 && y<=7)
        return 0;
    
    if(x>=12 && y<=7)
        return 0;
    
    if(x<=7 && y>=12)
        return 0;
    return 1;
}

void getData(char qrcode[20][20], char* data)
{
    size_t a = 0;
    for(size_t k = 0; k<5; k++)
    {
        for(size_t i = 20; i>=1; i--)
        {
            for(size_t j = 19-(k*4); j>=18-(k*4); j--)
            {
                if(isValid(i-1, j))
                {
                    data[a] = qrcode[i-1][j];
                    a++;
                }
            }
        }
        for(size_t i = 0; i<=19; i++)
        {
            for(size_t j = 17-(k*4)+1; j>=16-(k*4)+1; j--)
            {
                if(isValid(i, j-1))
                {
                    data[a] = qrcode[i][j-1];
                    a++;
                }
            }
        }
    }
}

size_t getEnc(char* data)
{
    return 8*data[0]+4*data[1]+2*data[2]+data[3];
}

size_t getLenth(char* data)
{
    return 128*data[4] 
        + 64*data[5] 
        + 32*data[6] 
        + 16*data[7] 
        + 8*data[8] 
        + 4*data[9] 
        + 2*data[10] 
        + data[11];
}

void getMessage(char* buffer, char* data, size_t len)
{
    for(size_t i = 0; i<len; i++)
    {
        buffer[i] = 128*data[i*8 + 12] 
                    + 64*data[i*8 + 13] 
                    + 32*data[i*8 + 14] 
                    + 16*data[i*8 + 15] 
                    + 8*data[i*8 + 16] 
                    + 4*data[i*8 + 17] 
                    + 2*data[i*8 + 18] 
                    + data[i*8 + 19];
    }
}
void stripQrCode(char qrcode[21][21], char new[20][20])
{
    size_t x = 0;
    size_t y = 0;
    for(size_t i = 0; i<21; i++)
    {
        for(size_t j = 0; j<21; j++)
        {
            x = i;
            y = j;
            if(i == 6 || j == 6)
                continue;

            else if(x>6)
                x = i-1;
            if(y>6)
                y = y-1;
            
            new[x][y] = qrcode[i][j];
        }
    }
}
int getMask(char qrcode[21][21])
{
    int a = qrcode[8][2] + 2*qrcode[8][3] + 4*qrcode[8][4];
    int b = qrcode[18][8] + 2*qrcode[17][8] + 4*qrcode[16][8];
    if(a!=b)
        errx(EXIT_FAILURE, "Can't read mask.\n");
    return a;
}

int main()
{
    char qrcode[21][21] = 
    {
        {1,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1},
        {1,0,1,1,1,0,1,0,0,0,0,1,1,0,1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1,0,0,0,0,1,1,0,1,0,1,1,1,0,1},
        {1,0,1,1,1,0,1,0,0,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,1,0,1,1,1,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
        {1,0,0,1,0,1,1,0,1,0,1,0,0,1,0,1,0,0,0,0,0},
        {1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,0,0,1},
        {1,1,0,0,0,1,1,1,1,1,0,0,1,0,1,1,0,0,1,0,1},
        {1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,1,0,0,1},
        {1,0,0,0,0,0,1,0,1,0,1,0,1,1,0,1,1,0,0,1,1},
        {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,0},
        {1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,1,1,0},
        {1,0,0,0,0,0,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,0,1,1,1,0},
        {1,0,1,1,1,0,1,0,1,1,0,1,0,0,0,1,1,0,0,1,1},
        {1,0,1,1,1,0,1,0,0,0,0,0,1,0,0,1,1,0,0,0,1},
        {1,0,0,0,0,0,1,0,0,0,1,0,0,1,1,0,1,0,0,0,0},
        {1,1,1,1,1,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0},
    };
    for(size_t i = 0; i<21; i++)
    {
        for(size_t j = 0; j<21; j++)
        {
            printf("%i,",(int)qrcode[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    char new[20][20];

    stripQrCode(qrcode, new);

    int mask = getMask(qrcode);

    addMask(mask, new);
    
    char* qr = malloc(sizeof(char)*208);
    getData(new, qr);

    for(size_t i = 0; i<208; i++)
        printf("%i", (int)qr[i]);

    printf("\n");

    size_t enc = getEnc(qr);
    size_t len = getLenth(qr);

    printf("Enc: %i\nLength: %i\nMask: %i\n", (int)enc, (int)len, mask);

    char buffer[len];
    getMessage(buffer, qr, len);

    printf("Message: ");
    for(size_t i = 0; i<len; i++)
        printf("%c", buffer[i]);

    printf("\n");

    free(qr);
    return EXIT_SUCCESS;
}