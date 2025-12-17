import type { Reference } from '../types'

interface ReferenceCardProps {
    reference: Reference
    onClick: (ref: Reference) => void
}

export function ReferenceCard({ reference, onClick }: ReferenceCardProps) {
    return (
        <button
            onClick={() => onClick(reference)}
            className="border rounded-lg p-3 hover:shadow-md hover:border-blue-400 transition-all text-left group"
        >
            <div className="relative overflow-hidden rounded mb-2">
                <img
                    src={reference.image}
                    alt={reference.code}
                    loading="lazy"
                    className="w-full h-24 md:h-32 object-cover group-hover:scale-105 transition-transform duration-200"
                />
            </div>
            <p className="font-semibold text-sm truncate">{reference.code}</p>
            <p className="text-xs text-gray-500 dark:text-gray-400 truncate">{reference.category}</p>
        </button>
    )
}
