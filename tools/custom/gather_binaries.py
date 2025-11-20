import glob
import shutil


def main():
    rels = glob.glob('./build/SOUE01/**/*.rel', recursive=True)
    for rel in rels:
        shutil.copy(rel, 'D:/ssrando/actual-extract/DATA/files/rels')

    shutil.copy('./build/SOUE01/main.dol', 'D:/ssrando/actual-extract/DATA/sys/')


if __name__ == "__main__":
    main()
