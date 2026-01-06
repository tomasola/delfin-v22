import type { Reference } from '../types'

interface ReferenceCardProps {
    reference: Reference
    onClick: (ref: Reference) => void
    onPrint: () => void
}

export function ReferenceCard({ reference, onClick, onPrint }: ReferenceCardProps) {
    return (
        <div
            className="border rounded-lg p-3 hover:shadow-md hover:border-blue-400 transition-all text-left group bg-white dark:bg-gray-800 relative"
        >
            <div
                onClick={() => onClick(reference)}
                className="cursor-pointer"
            >
                <div className="relative overflow-hidden rounded mb-2">
                    <img
                        src={reference.image}
                        alt={reference.code}
                        loading="lazy"
                        className="w-full h-24 md:h-32 object-cover group-hover:scale-105 transition-transform duration-200"
                    />
                </div>
                <p className="font-semibold text-sm truncate dark:text-white">{reference.code}</p>
                <p className="text-xs text-gray-500 dark:text-gray-400 truncate">{reference.category}</p>
            </div>

            <button
                onClick={(e) => {
                    e.stopPropagation();
                    onPrint();
                }}
                className="absolute top-2 right-2 p-2 bg-white/90 dark:bg-gray-800/90 rounded-full shadow-md hover:bg-white dark:hover:bg-gray-700 transition-colors border border-gray-100 dark:border-gray-600 group/print"
                title="Imprimir etiqueta"
            >
                <span className="text-lg group-hover/print:scale-110 transition-transform inline-block">🖨️</span>
            </button>
        </div>
    )
}
