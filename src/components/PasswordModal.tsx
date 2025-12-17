interface PasswordModalProps {
    isOpen: boolean
    onConfirm: () => void
    onCancel: () => void
}

export function PasswordModal({ isOpen, onConfirm, onCancel }: PasswordModalProps) {
    const [password, setPassword] = useState('')
    const [error, setError] = useState(false)

    if (!isOpen) return null

    const handleSubmit = (e: React.FormEvent) => {
        e.preventDefault()
        if (password === '1234') {
            onConfirm()
            setPassword('')
            setError(false)
        } else {
            setError(true)
        }
    }

    return (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center p-4 z-50 animate-fade-in">
            <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl p-6 w-full max-w-sm">
                <h3 className="text-xl font-bold mb-4 dark:text-white">Autenticación Requerida</h3>
                <p className="text-gray-600 dark:text-gray-400 mb-4">
                    Esta referencia ya contiene datos. Ingrese la contraseña para modificarla.
                </p>

                <form onSubmit={handleSubmit}>
                    <input
                        type="password"
                        value={password}
                        onChange={(e) => {
                            setPassword(e.target.value)
                            setError(false)
                        }}
                        placeholder="Contraseña"
                        className={`w-full px-4 py-2 border rounded-lg mb-2 focus:outline-none focus:ring-2 dark:bg-gray-700 dark:text-white ${error ? 'border-red-500 focus:ring-red-500' : 'border-gray-300 dark:border-gray-600 focus:ring-blue-500'
                            }`}
                        autoFocus
                    />
                    {error && <p className="text-red-500 text-sm mb-4">Contraseña incorrecta</p>}

                    <div className="flex justify-end gap-2 mt-4">
                        <button
                            type="button"
                            onClick={onCancel}
                            className="px-4 py-2 text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg"
                        >
                            Cancelar
                        </button>
                        <button
                            type="submit"
                            className="px-4 py-2 bg-blue-500 text-white hover:bg-blue-600 rounded-lg"
                        >
                            Confirmar
                        </button>
                    </div>
                </form>
            </div>
        </div>
    )
}

import { useState } from 'react'
import React from 'react'
