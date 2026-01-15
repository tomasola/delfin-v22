import type { Reference } from '../types';
import { RobustImage } from './RobustImage';

interface ImageEnlargedModalProps {
    reference: Reference;
    userCaptures: { embedding: number[], image: string }[];
    onClose: () => void;
    onViewDetail: () => void;
}

export function ImageEnlargedModal({ reference, userCaptures, onClose, onViewDetail }: ImageEnlargedModalProps) {
    return (
        <div className="fixed inset-0 z-50 bg-black/95 flex items-center justify-center p-4 animate-in fade-in duration-200">
            <div className="relative w-full max-w-4xl flex flex-col gap-4">
                {/* Header */}
                <div className="flex justify-between items-center">
                    <div>
                        <h2 className="text-2xl md:text-3xl font-bold text-white tracking-widest">{reference.code}</h2>
                        <p className="text-sm text-orange-400 font-bold uppercase tracking-wider mt-1">
                            🤖 {userCaptures.length} Foto{userCaptures.length !== 1 ? 's' : ''} IA Guardada{userCaptures.length !== 1 ? 's' : ''}
                        </p>
                    </div>
                    <button
                        onClick={onClose}
                        className="p-3 text-white text-2xl hover:bg-white/10 rounded-full transition-colors"
                    >
                        ✕
                    </button>
                </div>

                {/* Image Gallery */}
                <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                    {/* Official Image */}
                    <div className="bg-white rounded-2xl overflow-hidden shadow-2xl border-2 border-white/20">
                        <div className="aspect-square flex items-center justify-center p-4">
                            <RobustImage
                                code={reference.code}
                                className="w-full h-full object-contain"
                            />
                        </div>
                        <div className="bg-gray-900 text-white text-center py-2 px-4">
                            <p className="text-xs font-bold uppercase tracking-widest">🖼️ Catálogo Oficial</p>
                        </div>
                    </div>

                    {/* User Captures */}
                    {userCaptures.map((capture, idx) => (
                        <div key={idx} className="bg-gray-800 rounded-2xl overflow-hidden shadow-2xl border-2 border-orange-500/30">
                            <div className="aspect-square flex items-center justify-center">
                                <img
                                    src={capture.image}
                                    alt={`Captura IA ${idx + 1}`}
                                    className="w-full h-full object-cover"
                                />
                            </div>
                            <div className="bg-orange-600 text-white text-center py-2 px-4">
                                <p className="text-xs font-bold uppercase tracking-widest">🤖 Captura IA {idx + 1}</p>
                            </div>
                        </div>
                    ))}
                </div>

                {/* Actions */}
                <div className="flex gap-3 justify-center">
                    <button
                        onClick={onClose}
                        className="px-6 py-3 bg-gray-800 hover:bg-gray-700 text-white rounded-xl font-bold text-sm border border-white/10 active:scale-95 transition-all uppercase tracking-widest flex items-center gap-2"
                    >
                        <span>⬅️</span> Cerrar
                    </button>
                    <button
                        onClick={onViewDetail}
                        className="px-6 py-3 bg-blue-600 hover:bg-blue-500 text-white rounded-xl font-bold text-sm border border-blue-400/50 shadow-[0_0_20px_rgba(37,99,235,0.4)] active:scale-95 transition-all uppercase tracking-widest flex items-center gap-2"
                    >
                        <span>📋</span> Ver Ficha Completa
                    </button>
                </div>
            </div>
        </div>
    );
}
