/*
* This file is part of buteo-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, 
* this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may 
* be used to endorse or promote products derived from this software without 
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#ifndef LOGMACROS_H
#define LOGMACROS_H

#include <QString>
#include <QTime>
#include <QDebug>

//! Helper macro for writing log messages.
#ifndef QT_NO_DEBUG
#define LOG_MSG_L(level, msg) (QDebug((QtMsgType)(level)) << __FILE__ << __LINE__ << ":" << msg)
#define LOG_MSG_L_PLAIN(level, msg) (QDebug((QtMsgType)(level)) << msg)
#define QDEBUG(...) qDebug(__VA_ARGS__)
#else
#define LOG_MSG_L(level, msg)
#define LOG_MSG_L_PLAIN(level, msg)
#define QDEBUG(...)
#endif

// This macro is defined in both debug and release builds.
#define LOG_MSG_L_ALWAYS(level, msg) (QDebug((QtMsgType)(level)) << __FILE__ << __LINE__ << ":" << msg)

//! Macros for writing log messages. Use these.
//! Messages with level below warning are enabled only in debug builds.
#define LOG_FATAL(msg) LOG_MSG_L_ALWAYS(QtFatalMsg, msg)
#define LOG_CRITICAL(msg) LOG_MSG_L_ALWAYS(QtCriticalMsg, msg)
#define LOG_WARNING(msg) LOG_MSG_L_ALWAYS(QtWarningMsg, msg)
#define LOG_PROTOCOL(msg) LOG_MSG_L(QtDebugMsg, msg)
#define LOG_INFO(msg) LOG_MSG_L(QtDebugMsg, msg)
#define LOG_DEBUG(msg) LOG_MSG_L(QtDebugMsg, msg)
#define LOG_TRACE(msg) LOG_MSG_L(QtDebugMsg, msg)
#define LOG_TRACE_PLAIN(msg) LOG_MSG_L_PLAIN(QtDebugMsg, msg)

/*!
 * Creates a trace message to log when the function is entered and exited.
 * Logs also to time spent in the function.
 */
#ifndef QT_NO_DEBUG
#define FUNCTION_CALL_TRACE LogTimer timerDebugVariable(QString(__PRETTY_FUNCTION__));
#else
#define FUNCTION_CALL_TRACE
#endif


/*!
 * \brief Helper class for timing function execution time.
 */
class LogTimer
{
public:
    /*!
    * \brief Constructor. Creates an entry message to the log.
    *
    * @param aFunc Name of the function.
    */
    LogTimer(const QString &aFunc) : iFunc(aFunc)
    {
        LOG_TRACE_PLAIN(iFunc << ":Entry");
        iTimer.start();
    }

    /*!
     * \brief Destructor. Creates an exit message to the log, including
     *        function execution time.
     */
    ~LogTimer()
    {
        LOG_TRACE_PLAIN(iFunc << ":Exit, execution time:" << iTimer.elapsed()
            << "ms");
    }

private:
    QTime iTimer;
    QString iFunc;
};

#endif // LOGMACROS_H
