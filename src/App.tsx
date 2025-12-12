import { useState, useEffect } from 'react'

interface Reference {
  code: string
  image: string
  category: string
}

interface ReferenceData {
  length: string
  quantity: string
  boxSize: string
  notes: string
}

function App() {
  const [search, setSearch] = useState('')
  const [references, setReferences] = useState<Reference[]>([])
  const [results, setResults] = useState<Reference[]>([])
  const [selectedRef, setSelectedRef] = useState<Reference | null>(null)
  const [refData, setRefData] = useState<ReferenceData>({
    length: '',
    quantity: '',
    boxSize: '',
    notes: ''
  })

  useEffect(() => {
    fetch('/references.json')
      .then(res => res.json())
      .then(data => setReferences(data))
      .catch(err => console.error('Error:', err))
  }, [])

  useEffect(() => {
    if (search.trim() === '') {
      setResults([])
      return
    }
    const filtered = references.filter(ref => 
      ref.code.toLowerCase().includes(search.toLowerCase())
    )
    setResults(filtered.slice(0, 20))
  }, [search, references])

  const handleSelectRef = (ref: Reference) => {
    setSelectedRef(ref)
    const saved = localStorage.getItem(`ref_${ref.code}`)
    if (saved) {
      setRefData(JSON.parse(saved))
    } else {
      setRefData({ length: '', quantity: '', boxSize: '', notes: '' })
    }
  }

  const handleSaveData = () => {
    if (selectedRef) {
      localStorage.setItem(`ref_${selectedRef.code}`, JSON.stringify(refData))
      alert('Datos guardados')
    }
  }

  if (selectedRef) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 p-4">
        <div className="max-w-2xl mx-auto">
          <button
            onClick={() => setSelectedRef(null)}
            className="mb-4 text-blue-600 flex items-center gap-2"
          >
            ← Volver
          </button>
          
          <div className="bg-white rounded-lg shadow-lg p-6">
            <img 
              src={selectedRef.image} 
              alt={selectedRef.code}
              className="w-full max-h-64 object-contain rounded mb-4"
            />
            <h2 className="text-2xl font-bold mb-2">{selectedRef.code}</h2>
            <p className="text-gray-600 mb-6">{selectedRef.category}</p>
            
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-1">Longitud (m)</label>
                <input
                  type="number"
                  value={refData.length}
                  onChange={(e) => setRefData({...refData, length: e.target.value})}
                  className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none"
                  placeholder="0"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium mb-1">Cantidad</label>
                <input
                  type="number"
                  value={refData.quantity}
                  onChange={(e) => setRefData({...refData, quantity: e.target.value})}
                  className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none"
                  placeholder="0"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium mb-1">Tamaño de caja</label>
                <input
                  type="text"
                  value={refData.boxSize}
                  onChange={(e) => setRefData({...refData, boxSize: e.target.value})}
                  className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none"
                  placeholder="Ej: 50x30x20"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium mb-1">Notas</label>
                <textarea
                  value={refData.notes}
                  onChange={(e) => setRefData({...refData, notes: e.target.value})}
                  className="w-full px-4 py-2 border rounded-lg focus:ring-2 focus:ring-blue-500 focus:outline-none"
                  rows={3}
                  placeholder="Notas adicionales..."
                />
              </div>
              
              <button
                onClick={handleSaveData}
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

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 p-4">
      <div className="max-w-4xl mx-auto">
        <div className="text-center mb-8 mt-4">
          <img src="/logo.webp" alt="Delfín" className="w-20 h-20 mx-auto mb-4" />
          <h1 className="text-3xl md:text-4xl font-bold text-gray-800">
            Delfín Etiquetas
          </h1>
        </div>
        
        <div className="relative mb-8">
          <input
            type="text"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            placeholder="Buscar referencia..."
            className="w-full px-6 py-4 text-lg rounded-full shadow-lg border-2 border-transparent focus:border-blue-500 focus:outline-none transition-all"
          />
          {search && (
            <button
              onClick={() => setSearch('')}
              className="absolute right-4 top-1/2 -translate-y-1/2 text-gray-400 hover:text-gray-600 text-2xl"
            >
              ✕
            </button>
          )}
        </div>

        {results.length > 0 && (
          <div className="bg-white rounded-lg shadow-lg p-4 md:p-6">
            <p className="text-sm text-gray-600 mb-4">
              {results.length} resultado{results.length !== 1 ? 's' : ''}
            </p>
            <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-3 md:gap-4">
              {results.map((ref) => (
                <button
                  key={ref.code}
                  onClick={() => handleSelectRef(ref)}
                  className="border rounded-lg p-3 hover:shadow-md transition-shadow text-left"
                >
                  <img 
                    src={ref.image} 
                    alt={ref.code}
                    className="w-full h-24 md:h-32 object-cover rounded mb-2"
                  />
                  <p className="font-semibold text-sm truncate">{ref.code}</p>
                  <p className="text-xs text-gray-500 truncate">{ref.category}</p>
                </button>
              ))}
            </div>
          </div>
        )}

        {search && results.length === 0 && references.length > 0 && (
          <div className="text-center text-gray-600">
            <p>No se encontraron resultados</p>
          </div>
        )}
      </div>
    </div>
  )
}

export default App