import { useEffect } from 'react'
import type { ToastMessage } from '../types'

interface ToastProps {
    message: ToastMessage
    onClose: (id: string) => void
}

export function Toast({ message, onClose }: ToastProps) {
    useEffect(() => {
        const duration = message.duration || 3000
        const timer = setTimeout(() => {
            onClose(message.id)
        }, duration)

        return () => clearTimeout(timer)
    }, [message, onClose])

    const bgColors = {
        success: 'bg-green-500',
        error: 'bg-red-500',
        warning: 'bg-yellow-500',
        info: 'bg-blue-500'
    }

    const icons = {
        success: '✓',
        error: '✕',
        warning: '⚠',
        info: 'ℹ'
    }

    return (
        <div className={`${bgColors[message.type]} text-white px-6 py-4 rounded-lg shadow-lg flex items-center gap-3 min-w-[300px] animate-slide-in`}>
            <span className="text-2xl">{icons[message.type]}</span>
            <p className="flex-1">{message.message}</p>
            <button
                onClick={() => onClose(message.id)}
                className="text-white hover:text-gray-200 text-xl"
                aria-label="Cerrar notificación"
            >
                ✕
            </button>
        </div>
    )
}

interface ToastContainerProps {
    messages: ToastMessage[]
    onClose: (id: string) => void
}

export function ToastContainer({ messages, onClose }: ToastContainerProps) {
    return (
        <div className="fixed top-4 right-4 z-50 flex flex-col gap-2">
            {messages.map(message => (
                <Toast key={message.id} message={message} onClose={onClose} />
            ))}
        </div>
    )
}
