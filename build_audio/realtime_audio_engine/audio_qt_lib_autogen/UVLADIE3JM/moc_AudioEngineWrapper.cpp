/****************************************************************************
** Meta object code from reading C++ file 'AudioEngineWrapper.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../realtime_audio_engine/src/AudioEngineWrapper.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AudioEngineWrapper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN18AudioEngineWrapperE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioEngineWrapper::qt_create_metaobjectdata<qt_meta_tag_ZN18AudioEngineWrapperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioEngineWrapper",
        "QML.Element",
        "auto",
        "cutoffHzChanged",
        "",
        "overrunsChanged",
        "waveformUpdated",
        "setCutoffHz",
        "hz",
        "onPollTimer",
        "savePreset",
        "name",
        "loadPreset",
        "id",
        "cutoffHz",
        "overruns",
        "waveformBuffer",
        "QList<float>",
        "presetModel",
        "QAbstractListModel*"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'cutoffHzChanged'
        QtMocHelpers::SignalData<void()>(3, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'overrunsChanged'
        QtMocHelpers::SignalData<void()>(5, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'waveformUpdated'
        QtMocHelpers::SignalData<void()>(6, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setCutoffHz'
        QtMocHelpers::SlotData<void(float)>(7, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 8 },
        }}),
        // Slot 'onPollTimer'
        QtMocHelpers::SlotData<void()>(9, 4, QMC::AccessPrivate, QMetaType::Void),
        // Method 'savePreset'
        QtMocHelpers::MethodData<void(const QString &)>(10, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'loadPreset'
        QtMocHelpers::MethodData<void(int)>(12, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'cutoffHz'
        QtMocHelpers::PropertyData<float>(14, QMetaType::Float, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'overruns'
        QtMocHelpers::PropertyData<int>(15, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'waveformBuffer'
        QtMocHelpers::PropertyData<QList<float>>(16, 0x80000000 | 17, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'presetModel'
        QtMocHelpers::PropertyData<QAbstractListModel*>(18, 0x80000000 | 19, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<AudioEngineWrapper, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject AudioEngineWrapper::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AudioEngineWrapperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AudioEngineWrapperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18AudioEngineWrapperE_t>.metaTypes,
    nullptr
} };

void AudioEngineWrapper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioEngineWrapper *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->cutoffHzChanged(); break;
        case 1: _t->overrunsChanged(); break;
        case 2: _t->waveformUpdated(); break;
        case 3: _t->setCutoffHz((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 4: _t->onPollTimer(); break;
        case 5: _t->savePreset((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->loadPreset((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioEngineWrapper::*)()>(_a, &AudioEngineWrapper::cutoffHzChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngineWrapper::*)()>(_a, &AudioEngineWrapper::overrunsChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngineWrapper::*)()>(_a, &AudioEngineWrapper::waveformUpdated, 2))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractListModel* >(); break;
        case 2:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<float> >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<float*>(_v) = _t->cutoffHz(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->overruns(); break;
        case 2: *reinterpret_cast<QList<float>*>(_v) = _t->waveformBuffer(); break;
        case 3: *reinterpret_cast<QAbstractListModel**>(_v) = _t->presetModel(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setCutoffHz(*reinterpret_cast<float*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *AudioEngineWrapper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioEngineWrapper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AudioEngineWrapperE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioEngineWrapper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void AudioEngineWrapper::cutoffHzChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AudioEngineWrapper::overrunsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AudioEngineWrapper::waveformUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
