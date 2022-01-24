#include "ffmpegs.h"
#include "QDebug"

#include <QFile>

FFmpegs::FFmpegs()
{

}


void FFmpegs::pcm2wav(WAVHeader &header, const char *pcmFilePath, const char *wavFilePath){

    qDebug() << pcmFilePath << wavFilePath;


    header.blockAlign = (header.bitsPerSample * header.numChannels) >> 3;
    header.byteRate = header.blockAlign * header.sampleRate;


    QFile pcmFile(pcmFilePath);
    if(pcmFile.open(QFile::ReadOnly) == 0){

        qDebug() << "open pcm file error:" << pcmFilePath;
       return;
    }

    header.dataChunkDataSize = pcmFile.size();
    header.riffChunkDataSize = header.dataChunkDataSize + sizeof (WAVHeader) - sizeof (header.riffChunkId) - sizeof (header.riffChunkDataSize);

    QFile wavFile(wavFilePath);
    if(wavFile.open(QFile::WriteOnly) == 0){

       qDebug() << "open wav file error:" << wavFilePath;
       pcmFile.close();
       return;
    }


    // 写入header
    wavFile.write((const char *)&header,sizeof (header));

    u_int32_t pcmSize = 0;
    char data[1024];
    qint64 size = 0;
    while((size = pcmFile.read(data,sizeof (data))) > 0){
        wavFile.write(data, size);
        pcmSize += size;
    }
    qDebug()<<"pcm size" << pcmSize << pcmFile.size();

    qDebug()<< "wav生成成功：" << wavFilePath;


    pcmFile.close();
    wavFile.close();
}
