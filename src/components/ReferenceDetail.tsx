import { useState, useEffect } from 'react'
import type { Reference, ReferenceData } from '../types'
import { getReferenceData, saveReferenceData } from '../services/db'
import { PasswordModal } from './PasswordModal'

interface ReferenceDetailProps {
    reference: Reference
    onBack: () => void
    onSave: (success: boolean) => void
}

export function ReferenceDetail({ reference, onBack, onSave }: ReferenceDetailProps) {
    const [refData, setRefData] = useState<ReferenceData>({ length: '', quantity: '', boxSize: '', notes: '' })
    const [originalData, setOriginalData] = useState<ReferenceData | null>(null)
    const [loading, setLoading] = useState(true)
    const [showPasswordModal, setShowPasswordModal] = useState(false)
    const [errors, setErrors] = useState<Record<string, string>>({})

    useEffect(() => {
        const loadData = async () => {
            setLoading(true)
            const data = await getReferenceData(reference.code)
            if (data) {
                setRefData(data)
                setOriginalData(data)
            } else {
                setRefData({ length: '', quantity: '', boxSize: '', notes: '' })
                setOriginalData(null)
            }
            setLoading(false)
        }
        loadData()
    }, [reference.code])

    const validateForm = (): boolean => {
        const newErrors: Record<string, string> = {}

        if (refData.length && parseFloat(refData.length) <= 0) {
            newErrors.length = 'La longitud debe ser mayor a 0'
        }

        if (refData.quantity && parseInt(refData.quantity) <= 0) {
            newErrors.quantity = 'La cantidad debe ser mayor a 0'
        }

        setErrors(newErrors)
        return Object.keys(newErrors).length === 0
    }

    const handleSaveClick = () => {
        if (!validateForm()) {
            onSave(false)
            return
        }

        // Check if data existed previously (is not a new entry)
        const hasExistingData = originalData && (
            originalData.length || originalData.quantity || originalData.boxSize || originalData.notes
        )

        if (hasExistingData) {
            setShowPasswordModal(true)
        } else {
            performSave()
        }
    }

    const performSave = async () => {
        const success = await saveReferenceData(reference.code, refData)
        if (success) setOriginalData(refData)
        setShowPasswordModal(false)
        onSave(success)
    }

    if (loading) {
        return (
            <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 dark:from-gray-900 dark:to-gray-800 p-4 flex items-center justify-center">
                <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
                    <p className="text-gray-600 dark:text-gray-400">Cargando datos...</p>
                </div>
            </div>
        )
    }

    return (
        <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 dark:from-gray-900 dark:to-gray-800 p-4">
            <PasswordModal
                isOpen={showPasswordModal}
                onConfirm={performSave}
                onCancel={() => setShowPasswordModal(false)}
            />

            <div className="max-w-2xl mx-auto">
                <button
                    onClick={onBack}
                    className="mb-6 px-6 py-3 bg-white dark:bg-gray-800 text-gray-700 dark:text-gray-200 rounded-xl shadow-md hover:shadow-lg hover:bg-gray-50 dark:hover:bg-gray-700 transition-all font-bold flex items-center gap-2 border border-gray-100 dark:border-gray-700 animate-in slide-in-from-left duration-300"
                >
                    <span className="text-xl">⬅️</span> Volver al listado
                </button>

                <div className="bg-white dark:bg-gray-800 rounded-lg shadow-lg p-6">
                    <img
                        src={reference.image}
                        alt={reference.code}
                        loading="lazy"
                        className="w-full max-h-64 object-contain rounded mb-4"
                    />
                    <h2 className="text-2xl font-bold mb-2 dark:text-white">{reference.code}</h2>
                    <p className="text-gray-600 dark:text-gray-400 mb-6">{reference.category}</p>

                    <div className="space-y-4">
                        <div>
                            <label className="block text-sm font-medium mb-1 dark:text-gray-300">Longitud (m)</label>
                            <input
                                type="number"
                                value={refData.length}
                                onChange={(e) => setRefData({ ...refData, length: e.target.value })}
                                className={`w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none dark:bg-gray-700 dark:text-white dark:border-gray-600 ${errors.length ? 'border-red-500' : ''
                                    }`}
                                placeholder="0"
                            />
                            {errors.length && <p className="text-red-500 text-sm mt-1">{errors.length}</p>}
                        </div>

                        <div>
                            <label className="block text-sm font-medium mb-1 dark:text-gray-300">Cantidad</label>
                            <input
                                type="number"
                                value={refData.quantity}
                                onChange={(e) => setRefData({ ...refData, quantity: e.target.value })}
                                className={`w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none dark:bg-gray-700 dark:text-white dark:border-gray-600 ${errors.quantity ? 'border-red-500' : ''
                                    }`}
                                placeholder="0"
                            />
                            {errors.quantity && <p className="text-red-500 text-sm mt-1">{errors.quantity}</p>}
                        </div>

                        <div>
                            <label className="block text-sm font-medium mb-1 dark:text-gray-300">Tamaño de caja</label>
                            <input
                                type="text"
                                value={refData.boxSize}
                                onChange={(e) => setRefData({ ...refData, boxSize: e.target.value })}
                                className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none dark:bg-gray-700 dark:text-white dark:border-gray-600"
                                placeholder="Ej: 50x30x20"
                            />
                        </div>

                        <div>
                            <label className="block text-sm font-medium mb-1 dark:text-gray-300">Notas</label>
                            <textarea
                                value={refData.notes}
                                onChange={(e) => setRefData({ ...refData, notes: e.target.value })}
                                className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none dark:bg-gray-700 dark:text-white dark:border-gray-600"
                                rows={3}
                                placeholder="Notas adicionales..."
                            />
                        </div>

                        <button
                            onClick={handleSaveClick}
                            className="w-full bg-blue-500 hover:bg-blue-600 text-white py-3 rounded-lg font-medium transition-colors"
                        >
                            Guardar
                        </button>
                    </div>
                </div>
            </div>
        </div>
    )
}
